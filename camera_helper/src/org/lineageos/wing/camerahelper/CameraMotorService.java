/*
 * SPDX-FileCopyrightText: 2019 The LineageOS Project
 * SPDX-FileCopyrightText: 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.wing.camerahelper;

import android.annotation.NonNull;
import android.app.Service;
import android.content.Intent;
import android.hardware.camera2.CameraManager;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.util.Log;

public final class CameraMotorService extends Service {
    private static final String TAG = "WingCameraHelper";
    private static final String FRONT_CAMERA_ID = "1";
    private static final long CAMERA_EVENT_DELAY_MILLIS = 100;

    private final Handler mHandler = new Handler(Looper.getMainLooper());
    private final Runnable mRetractCamera = () -> setMotorPosition(false);
    private final Runnable mExtendCamera = () -> setMotorPosition(true);

    private final CameraManager.AvailabilityCallback mAvailabilityCallback =
            new CameraManager.AvailabilityCallback() {
                @Override
                public void onCameraAvailable(@NonNull String cameraId) {
                    if (FRONT_CAMERA_ID.equals(cameraId)) {
                        scheduleMotorAction(mRetractCamera);
                    }
                }

                @Override
                public void onCameraUnavailable(@NonNull String cameraId) {
                    if (FRONT_CAMERA_ID.equals(cameraId)) {
                        scheduleMotorAction(mExtendCamera);
                    }
                }
            };

    @Override
    public void onCreate() {
        super.onCreate();
        getSystemService(CameraManager.class).registerAvailabilityCallback(
                mAvailabilityCallback, mHandler);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        return START_STICKY;
    }

    @Override
    public void onDestroy() {
        getSystemService(CameraManager.class).unregisterAvailabilityCallback(
                mAvailabilityCallback);
        mHandler.removeCallbacksAndMessages(null);
        setMotorPosition(false);
        super.onDestroy();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    private void scheduleMotorAction(Runnable action) {
        mHandler.removeCallbacks(mRetractCamera);
        mHandler.removeCallbacks(mExtendCamera);
        mHandler.postDelayed(action, CAMERA_EVENT_DELAY_MILLIS);
    }

    private void setMotorPosition(boolean extended) {
        try {
            LgeMotor.setParameters(extended ? "popup_cam=on" : "popup_cam=off");
        } catch (Exception e) {
            Log.e(TAG, "Failed to " + (extended ? "extend" : "retract")
                    + " popup camera", e);
        }
    }
}
