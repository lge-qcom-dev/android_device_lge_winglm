#
# Copyright (C) 2025 The LineageOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

DEVICE_PATH := device/lge/winglm

DEVICE_NAME := winglm

# Inherit from the common device configuration.
$(call inherit-product, device/lge/sm7250-common/sm7250-common.mk)

# Audio
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/audio/audio_platform_info.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_platform_info.xml \
    $(LOCAL_PATH)/audio/audio_platform_info.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_platform_info_intcodec.xml \
    $(LOCAL_PATH)/audio/audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_policy_configuration.xml \
    $(LOCAL_PATH)/audio/mixer_paths.xml:$(TARGET_COPY_OUT_VENDOR)/etc/mixer_paths.xml

# Fingerprint
PRODUCT_PACKAGES += \
    android.hardware.biometrics.fingerprint@2.3-service.lge \
    sensors.lge

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/sensors/hals.conf:$(TARGET_COPY_OUT_VENDOR)/etc/sensors/hals.conf

$(call soong_config_set,LGE_FINGERPRINT_HAL,TARGET_HAS_EGISTEC_UDFPS,true)

# Overlays
PRODUCT_PACKAGES += \
    ApertureOverlayWinglm \
    FrameworksResOverlayWinglm \
    SettingsOverlayWinglm \
    SystemUIOverlayWinglm

# Soong namespace
PRODUCT_SOONG_NAMESPACES += \
    $(LOCAL_PATH)

# Inherit from vendor makefiles.
$(call inherit-product, vendor/lge/winglm/winglm-vendor.mk)
