# Bangbang

Android diagnostic HUD scaffold for ARM64 Unity applications.

## Included components
- Native memory mapping + offset readers (`/proc/self/maps` parser, anchor offset reads).
- JNI bridge with `DataPacket` (String + int + float).
- `HudOverlayService` that renders transparent overlay updates every 500 ms.
- `AndroidManifest.xml` permissions/services and NDK `CMakeLists.txt`.

## Build notes (Termux/AIDE)
1. Place this `app/` tree in your Android project.
2. Ensure your Gradle module enables external native build using `app/CMakeLists.txt`.
3. Grant overlay permission manually in Android settings for your app.
