/*
 * Copyright (C) 2016 The Android Open Source Project
 * SPDX-FileCopyrightText: 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.wing.camerahelper;

import android.hidl.base.V1_0.DebugInfo;
import android.hidl.base.V1_0.IBase;
import android.os.HidlSupport;
import android.os.HwBinder;
import android.os.HwBlob;
import android.os.HwParcel;
import android.os.IHwBinder;
import android.os.IHwInterface;
import android.os.RemoteException;

import java.util.ArrayList;
import java.util.Arrays;

final class MotorCallback extends HwBinder implements IHwInterface {
    private static final String INTERFACE =
            "vendor.lge.hardware.motor@1.0::ILgeMotorCallback";

    private static final byte[][] HASH_CHAIN = {
        {(byte) 0x8d, 0x52, 0x1d, (byte) 0xdf, (byte) 0xaa, (byte) 0xb0, (byte) 0xc6,
                (byte) 0xc7, (byte) 0x82, (byte) 0xbc, 0x5f, (byte) 0xa0, (byte) 0xc4,
                (byte) 0xac, 0x26, 0x1e, 0x27, (byte) 0xf2, (byte) 0x83, (byte) 0xf0,
                (byte) 0xd9, 0x3a, (byte) 0x93, (byte) 0xbf, (byte) 0xe6, 0x13, 0x4b,
                0x6c, (byte) 0xcd, (byte) 0xb9, 0x53, (byte) 0x9c},
        {(byte) 0xec, 0x7f, (byte) 0xd7, (byte) 0x9e, (byte) 0xd0, 0x2d, (byte) 0xfa,
                (byte) 0x85, (byte) 0xbc, 0x49, (byte) 0x94, 0x26, (byte) 0xad,
                (byte) 0xae, 0x3e, (byte) 0xbe, 0x23, (byte) 0xef, 0x05, 0x24,
                (byte) 0xf3, (byte) 0xcd, 0x69, 0x57, 0x13, (byte) 0x93, 0x24,
                (byte) 0xb8, 0x3b, 0x18, (byte) 0xca, 0x4c},
    };

    @Override
    public IHwBinder asBinder() {
        return this;
    }

    @Override
    public boolean linkToDeath(DeathRecipient recipient, long cookie) {
        return true;
    }

    @Override
    public IHwInterface queryLocalInterface(String descriptor) {
        return INTERFACE.equals(descriptor) ? this : null;
    }

    @Override
    public boolean unlinkToDeath(DeathRecipient recipient) {
        return true;
    }

    @Override
    public void onTransact(int code, HwParcel request, HwParcel reply, int flags)
            throws RemoteException {
        switch (code) {
            case 1:
                request.enforceInterface(INTERFACE);
                request.readInt32();
                break;
            case 256067662:
                request.enforceInterface(IBase.kInterfaceName);
                reply.writeStatus(0);
                reply.writeStringVector(new ArrayList<>(
                        Arrays.asList(INTERFACE, IBase.kInterfaceName)));
                reply.send();
                break;
            case 256131655:
                request.enforceInterface(IBase.kInterfaceName);
                request.readNativeHandle();
                request.readStringVector();
                reply.writeStatus(0);
                reply.send();
                break;
            case 256136003:
                request.enforceInterface(IBase.kInterfaceName);
                reply.writeStatus(0);
                reply.writeString(INTERFACE);
                reply.send();
                break;
            case 256398152:
                request.enforceInterface(IBase.kInterfaceName);
                reply.writeStatus(0);
                writeHashChain(reply);
                reply.send();
                break;
            case 256462420:
                request.enforceInterface(IBase.kInterfaceName);
                break;
            case 256921159:
                request.enforceInterface(IBase.kInterfaceName);
                reply.writeStatus(0);
                reply.send();
                break;
            case 257049926:
                request.enforceInterface(IBase.kInterfaceName);
                DebugInfo info = new DebugInfo();
                info.pid = HidlSupport.getPidIfSharable();
                info.ptr = 0;
                info.arch = 0;
                reply.writeStatus(0);
                info.writeToParcel(reply);
                reply.send();
                break;
            case 257120595:
                request.enforceInterface(IBase.kInterfaceName);
                HwBinder.enableInstrumentation();
                break;
        }
    }

    private static void writeHashChain(HwParcel reply) {
        HwBlob header = new HwBlob(16);
        header.putInt32(8, HASH_CHAIN.length);
        header.putBool(12, false);

        HwBlob hashes = new HwBlob(HASH_CHAIN.length * 32);
        for (int i = 0; i < HASH_CHAIN.length; i++) {
            hashes.putInt8Array(i * 32L, HASH_CHAIN[i]);
        }

        header.putBlob(0, hashes);
        reply.writeBuffer(header);
    }
}
