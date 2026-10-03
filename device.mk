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
$(call soong_config_set,lge_udfps,sensor_x,540)
$(call soong_config_set,lge_udfps,sensor_y,2005)
$(call soong_config_set,lge_udfps,sensor_radius,97)
$(call soong_config_set_bool,lge_udfps,managed_sequence,true)

$(call inherit-product, hardware/lge/aidl/biometrics/fingerprint/udfps.mk)
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

# Sensors
PRODUCT_PACKAGES += \
    sensors.lge

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/sensors/hals.conf:$(TARGET_COPY_OUT_VENDOR)/etc/sensors/hals.conf

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
