package com.example.diagnostics;

public final class NativeBridge {
    static {
        System.loadLibrary("diag_native");
    }

    private NativeBridge() {
    }

    public static native String getHudSnapshot(String libraryName, long pointerAnchor);

    public static native DataPacket enrichPacket(DataPacket packet);
}
