/*
 * Copyright (C) 2019 The Android Open Source Project
 * Copyright (C) 2019-2023 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "Vibrator.h"

#include <android-base/logging.h>

#include <cutils/properties.h>
#include <algorithm>
#include <cmath>
#include <thread>
#include <vector>

#include <linux/tspdrv.h>

namespace aidl {
namespace android {
namespace hardware {
namespace vibrator {

static constexpr uint8_t DEFAULT_AMPLITUDE = UINT8_MAX / 2;
static constexpr int32_t SAMPLES_PER_FRAME = 60;
static constexpr int32_t PREBUFFER_FRAMES = 3;
static constexpr double SAMPLES_PER_MS = 12.0;
static constexpr double SAMPLES_PER_RADIAN = 12.276305267;

// Click effect in ms
static constexpr int32_t WAVEFORM_CLICK_EFFECT_MS = 6;

// Tick effect in ms
static constexpr int32_t WAVEFORM_TICK_EFFECT_MS = 2;

// Double click effect in ms
static constexpr uint32_t WAVEFORM_DOUBLE_CLICK_EFFECT_MS = 135;

// Heavy click effect in ms
static constexpr uint32_t WAVEFORM_HEAVY_CLICK_EFFECT_MS = 8;

ndk::ScopedAStatus Vibrator::getCapabilities(int32_t* _aidl_return) {
    LOG(VERBOSE) << "Vibrator reporting capabilities";
    *_aidl_return = IVibrator::CAP_ON_CALLBACK | IVibrator::CAP_PERFORM_CALLBACK |
                    IVibrator::CAP_AMPLITUDE_CONTROL;
    return ndk::ScopedAStatus::ok();
}

Vibrator::Vibrator(int32_t file_desc, int32_t numActuators) {
    // Initialize default values for haptic motor
    mFile_desc = file_desc;
    mNumActuators = numActuators;

    mDefaultAmplitude = DEFAULT_AMPLITUDE;
    mCurrentAmplitude = mDefaultAmplitude;

    mClickDuration =
            property_get_int32("ro.vendor.vibrator.hal.click.duration", WAVEFORM_CLICK_EFFECT_MS);
    mTickDuration =
            property_get_int32("ro.vendor.vibrator.hal.tick.duration", WAVEFORM_TICK_EFFECT_MS);
    mHeavyClickDuration = property_get_int32("ro.vendor.vibrator.hal.heavyclick.duration",
                                             WAVEFORM_HEAVY_CLICK_EFFECT_MS);
}

ndk::ScopedAStatus Vibrator::off() {
    mSequence++;
    return stop();
}

ndk::ScopedAStatus Vibrator::stop() {
    for (int32_t i = 0; i < mNumActuators; i++) {
        int32_t ret = ioctl(mFile_desc, TSPDRV_DISABLE_AMP, i);
        if (ret != 0) {
            LOG(ERROR) << "Failed to deactivate Actuator with index " << i;
            return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_TRANSACTION_FAILED));
        }
    }

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::on(int32_t timeoutMs,
                                const std::shared_ptr<IVibratorCallback>& callback) {
    if (timeoutMs <= 0) {
        return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_ILLEGAL_ARGUMENT));
    }

    const uint64_t sequence = ++mSequence;
    auto status = stop();
    if (!status.isOk()) {
        return status;
    }

    std::thread(&Vibrator::play, this, timeoutMs, mCurrentAmplitude, sequence, callback).detach();
    return ndk::ScopedAStatus::ok();
}

void Vibrator::play(int32_t timeoutMs, uint8_t amplitude, uint64_t sequence,
                    const std::shared_ptr<IVibratorCallback>& callback) {
    const size_t sampleCount = static_cast<size_t>(round(SAMPLES_PER_MS * timeoutMs));
    const size_t frameCount = std::max(
            (sampleCount + SAMPLES_PER_FRAME - 1) / SAMPLES_PER_FRAME,
            static_cast<size_t>(PREBUFFER_FRAMES));

    for (int32_t i = 0; i < mNumActuators; i++) {
        if (mSequence != sequence) {
            return;
        }
        if (ioctl(mFile_desc, TSPDRV_ENABLE_AMP, i) != 0) {
            PLOG(ERROR) << "Failed to activate actuator " << i;
            return;
        }

        for (size_t frame = 0; frame < frameCount; frame++) {
            if (mSequence != sequence) {
                return;
            }
            std::vector<uint8_t> output(SAMPLES_PER_FRAME + SPI_HEADER_SIZE, 0);
            output[0] = i;
            output[1] = 8;
            output[2] = SAMPLES_PER_FRAME;
            for (int32_t sample = 0; sample < SAMPLES_PER_FRAME; sample++) {
                const size_t index = frame * SAMPLES_PER_FRAME + sample;
                if (index < sampleCount) {
                    output[sample + SPI_HEADER_SIZE] = static_cast<uint8_t>(
                            amplitude * sin(index / SAMPLES_PER_RADIAN));
                }
            }
            if (write(mFile_desc, output.data(), output.size()) !=
                static_cast<ssize_t>(output.size())) {
                PLOG(ERROR) << "Failed to write samples for actuator " << i;
                return;
            }
        }
    }

    if (mSequence == sequence && callback) {
        callback->onComplete();
    }
}

static uint8_t convertEffectStrength(EffectStrength strength, uint8_t defaultAmplitude) {
    uint8_t amplitude;

    switch (strength) {
        case EffectStrength::LIGHT:
            amplitude = defaultAmplitude / 2;
            break;
        case EffectStrength::MEDIUM:
            amplitude = defaultAmplitude;
            break;
        case EffectStrength::STRONG:
            amplitude = static_cast<uint8_t>(std::min(defaultAmplitude * 1.5, 127.0));
            break;
    }

    return amplitude;
}

ndk::ScopedAStatus Vibrator::perform(Effect effect, EffectStrength strength,
                                     const std::shared_ptr<IVibratorCallback>& callback,
                                     int32_t* _aidl_return) {
    ndk::ScopedAStatus status = ndk::ScopedAStatus::ok();
    uint32_t timeMS;
    uint8_t currentAmplitude;

    switch (effect) {
        case Effect::CLICK:
            timeMS = mClickDuration;
            break;
        case Effect::DOUBLE_CLICK:
            timeMS = WAVEFORM_DOUBLE_CLICK_EFFECT_MS;
            break;
        case Effect::TICK:
            timeMS = mTickDuration;
            break;
        case Effect::HEAVY_CLICK:
            timeMS = mHeavyClickDuration;
            break;
        default:
            *_aidl_return = 0;
            return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
    }

    // save current amplitude
    currentAmplitude = mCurrentAmplitude;

    mCurrentAmplitude = convertEffectStrength(strength, mDefaultAmplitude);
    status = on(timeMS, callback);

    // restore current amplitude
    mCurrentAmplitude = currentAmplitude;

    if (!status.isOk()) {
        *_aidl_return = 0;
        return status;
    }

    *_aidl_return = timeMS;
    return status;
}

ndk::ScopedAStatus Vibrator::getSupportedEffects(std::vector<Effect>* _aidl_return) {
    *_aidl_return = {Effect::CLICK, Effect::DOUBLE_CLICK, Effect::TICK, Effect::HEAVY_CLICK};
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::setAmplitude(float amplitude) {
    if (amplitude <= 0.0f || amplitude > 1.0f) {
        return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_ILLEGAL_ARGUMENT));
    }

    mCurrentAmplitude = static_cast<uint8_t>(amplitude * UINT8_MAX / 2);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::setExternalControl(bool /* enabled */) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getCompositionDelayMax(int32_t* /* maxDelayMs */) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getCompositionSizeMax(int32_t* /* maxSize */) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getSupportedPrimitives(
        std::vector<CompositePrimitive>* /* supported */) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getPrimitiveDuration(CompositePrimitive /* primitive */,
                                                  int32_t* /* durationMs */) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::compose(const std::vector<CompositeEffect>& /* composite */,
                                     const std::shared_ptr<IVibratorCallback>& /* callback */) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getSupportedAlwaysOnEffects(std::vector<Effect>* /* _aidl_return */) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::alwaysOnEnable(int32_t id, Effect /* effect */,
                                            EffectStrength /* strength */) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::alwaysOnDisable(int32_t /* id */) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getResonantFrequency(float* /* resonantFreqHz */) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getQFactor(float* /* qFactor */) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getFrequencyResolution(float* /* freqResolutionHz */) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getFrequencyMinimum(float* /* freqMinimumHz */) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getBandwidthAmplitudeMap(std::vector<float>* /* _aidl_return */) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getPwlePrimitiveDurationMax(int32_t* /* durationMs */) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getPwleCompositionSizeMax(int32_t* /* maxSize */) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getSupportedBraking(std::vector<Braking>* /* supported */) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::composePwle(const std::vector<PrimitivePwle>& /* composite */,
                                         const std::shared_ptr<IVibratorCallback>& /* callback */) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

}  // namespace vibrator
}  // namespace hardware
}  // namespace android
}  // namespace aidl
