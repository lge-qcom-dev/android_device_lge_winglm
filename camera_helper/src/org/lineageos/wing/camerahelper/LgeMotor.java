/*
 * SPDX-FileCopyrightText: 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.wing.camerahelper;

import android.os.HwBinder;
import android.os.HwParcel;
import android.os.IHwBinder;
import android.os.RemoteException;

final class LgeMotor {
    private static final String INTERFACE =
            "vendor.lge.hardware.motor@1.0::ILgeMotor";
    private static final String INSTANCE = "PopupCam";
    private static final int TRANSACTION_SET_PARAMETERS = 1;
    private static final int TRANSACTION_SET_CALLBACK = 3;

    private static final MotorCallback CALLBACK = new MotorCallback();

    private static IHwBinder sMotor;

    private LgeMotor() {}

    static synchronized void setParameters(String parameters) throws RemoteException {
        IHwBinder motor = getMotor();
        HwParcel request = new HwParcel();
        HwParcel reply = new HwParcel();

        try {
            request.writeInterfaceToken(INTERFACE);
            request.writeString(parameters);
            motor.transact(TRANSACTION_SET_PARAMETERS, request, reply, 0);
            reply.verifySuccess();
            request.releaseTemporaryStorage();
            reply.readInt32();
        } catch (RemoteException e) {
            sMotor = null;
            throw e;
        } finally {
            reply.release();
        }
    }

    private static IHwBinder getMotor() throws RemoteException {
        if (sMotor == null) {
            sMotor = HwBinder.getService(INTERFACE, INSTANCE);
            setCallback(sMotor);
        }
        return sMotor;
    }

    private static void setCallback(IHwBinder motor) throws RemoteException {
        HwParcel request = new HwParcel();
        HwParcel reply = new HwParcel();

        try {
            request.writeInterfaceToken(INTERFACE);
            request.writeStrongBinder(CALLBACK);
            motor.transact(TRANSACTION_SET_CALLBACK, request, reply, 1);
            request.releaseTemporaryStorage();
        } finally {
            reply.release();
        }
    }
}
