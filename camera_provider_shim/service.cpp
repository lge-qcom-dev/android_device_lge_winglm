/*
 * SPDX-FileCopyrightText: 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "WingCameraProviderShim"

#include <stdlib.h>

#include <binder/ProcessState.h>
#include <hidl/HidlTransportSupport.h>
#include <log/log.h>

#include <CameraProvider_2_4.h>
#include <LegacyCameraProviderImpl_2_4.h>

namespace android::hardware::camera::provider::V2_4::implementation {

namespace {

constexpr int kFirstBlockedCameraId = 4;
constexpr int kLastBlockedCameraId = 9;

struct WingCameraProviderImpl : public LegacyCameraProviderImpl_2_4 {
    WingCameraProviderImpl() : LegacyCameraProviderImpl_2_4() {
        camera_device_status_change = cameraDeviceStatusChange;
        torch_mode_status_change = torchModeStatusChange;

        // The LG HAL reports these malformed hidden devices synchronously from
        // set_callbacks(). Drop the statuses collected by the base constructor.
        for (int cameraId = kFirstBlockedCameraId;
                cameraId <= kLastBlockedCameraId; cameraId++) {
            mCameraStatusMap.erase(std::to_string(cameraId));
        }

        ALOGI("Filtering camera IDs %d through %d", kFirstBlockedCameraId,
                kLastBlockedCameraId);
    }

    static void cameraDeviceStatusChange(const camera_module_callbacks_t* callbacks,
            int cameraId, int newStatus) {
        if (cameraId >= kFirstBlockedCameraId) {
            ALOGI("Suppressing camera device status for ID %d", cameraId);
            return;
        }

        sCameraDeviceStatusChange(callbacks, cameraId, newStatus);
    }

    static void torchModeStatusChange(const camera_module_callbacks_t* callbacks,
            const char* cameraId, int newStatus) {
        char* end = nullptr;
        const long parsedId = cameraId == nullptr ? -1 : strtol(cameraId, &end, 10);
        if (cameraId != nullptr && end != cameraId && *end == '\0' &&
                parsedId >= kFirstBlockedCameraId) {
            ALOGI("Suppressing torch status for ID %s", cameraId);
            return;
        }

        sTorchModeStatusChange(callbacks, cameraId, newStatus);
    }
};

}  // namespace

}  // namespace android::hardware::camera::provider::V2_4::implementation

int main() {
    using android::OK;
    using android::ProcessState;
    using android::sp;
    using android::hardware::configureRpcThreadpool;
    using android::hardware::joinRpcThreadpool;
    using android::hardware::camera::provider::V2_4::ICameraProvider;
    using android::hardware::camera::provider::V2_4::implementation::CameraProvider;
    using android::hardware::camera::provider::V2_4::implementation::WingCameraProviderImpl;

    ProcessState::initWithDriver("/dev/vndbinder");
    configureRpcThreadpool(6, true);

    sp<ICameraProvider> provider = new CameraProvider<WingCameraProviderImpl>();
    if (provider->registerAsService("legacy/0") != OK) {
        ALOGE("Failed to register the filtered camera provider");
        return EXIT_FAILURE;
    }

    joinRpcThreadpool();
    return EXIT_FAILURE;
}
