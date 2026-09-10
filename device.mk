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
    libets_teeclient_v2_shim \
    sensors.lge

# Vibrator
PRODUCT_PACKAGES += \
    android.hardware.vibrator-service.winglm

# Display
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/display/display_settings.xml:$(TARGET_COPY_OUT_VENDOR)/etc/display_settings.xml \
    $(LOCAL_PATH)/configs/display/display_layout_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/displayconfig/display_layout_configuration.xml \
    $(LOCAL_PATH)/configs/display/device_state_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/devicestate/device_state_configuration.xml \
    $(LOCAL_PATH)/configs/idc/touch_sub_dev.idc:$(TARGET_COPY_OUT_VENDOR)/usr/idc/touch_sub_dev.idc

# Popup camera
PRODUCT_PACKAGES += \
    WingCameraHelper

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/sensors/hals.conf:$(TARGET_COPY_OUT_VENDOR)/etc/sensors/hals.conf

$(call soong_config_set,LGE_FINGERPRINT_HAL,TARGET_HAS_EGISTEC_UDFPS,true)

# Overlays
PRODUCT_PACKAGES += \
    ApertureOverlayWinglm \
    FrameworksResOverlayWinglm \
    SettingsOverlayWinglm \
    SystemUIOverlayWinglm

# Recovery
PRODUCT_SYSTEM_PROPERTIES += \
    ro.minui.blacklist_input_devices=winglm-swivel \
    ro.minui.default_rotation=ROTATION_NONE \
    ro.minui.default_touch_rotation=ROTATION_NONE

# Soong namespace
PRODUCT_SOONG_NAMESPACES += \
    $(LOCAL_PATH)

# Inherit from vendor makefiles.
$(call inherit-product, vendor/lge/winglm/winglm-vendor.mk)
