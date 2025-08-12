This is a Kotlin Multiplatform project targeting Desktop (JVM).

* [/composeApp](./composeApp/src) is for code that will be shared across your Compose Multiplatform applications.
  It contains several subfolders:
  - [commonMain](./composeApp/src/commonMain/kotlin) is for code that’s common for all targets.
  - Other folders are for Kotlin code that will be compiled for only the platform indicated in the folder name.
    For example, if you want to use Apple’s CoreCrypto for the iOS part of your Kotlin app,
    the [iosMain](./composeApp/src/iosMain/kotlin) folder would be the right place for such calls.
    Similarly, if you want to edit the Desktop (JVM) specific part, the [jvmMain](./composeApp/src/jvmMain/kotlin)
    folder is the appropriate location.


Learn more about [Kotlin Multiplatform](https://www.jetbrains.com/help/kotlin-multiplatform-dev/get-started.html)…

---

Android Screen Mirror Server (Sample)

A minimal Android-side screen mirroring server sample is provided at:
- samples/android-server/

It streams the device screen encoded via MediaCodec (Surface input) from a VirtualDisplay over a LocalServerSocket using a simple protocol:
- Header: 4 bytes ASCII codecId ("h264" | "h265" | "av1 "), 4 bytes width (BE u32), 4 bytes height (BE u32)
- Then packets: [8 bytes PTS/flags (BE u64)][4 bytes length (BE u32)][payload]
  - bit63 = 1 CONFIG buffer, bit62 = 1 KEY frame, bits[0..61] = presentationTimeUs if not CONFIG

Usage (high level):
- Integrate samples/android-server/ScreenServer.kt into an Android app/service and start ScreenServer(context, StreamOptions(...)).
- From desktop, create a tunnel to the Android local abstract socket, e.g.:
  adb -s <serial> forward tcp:27183 localabstract:scrcpy_k_demo
- Read the stream from localhost:27183 and parse as per the protocol above.

Note: The sample is not part of the desktop Gradle build to keep the project buildable without the Android SDK.

---

Android server (app_process) quickstart

Build the APK (to be used as a server jar):

```bash
./gradlew :androidServer:assembleRelease
cp androidServer/build/outputs/apk/release/androidServer-release.apk androidServer/build/dist/mirrordesk-android-server.jar
```

Push and run on device:

```bash
adb push androidServer/build/dist/mirrordesk-android-server.jar /data/local/tmp/mirrordesk-android-server.jar
adb forward tcp:27183 localabstract:mirrordesk_demo
adb shell CLASSPATH=/data/local/tmp/mirrordesk-android-server.jar \
  app_process / com.mirrordesk.androidserver.MiniServer --socket mirrordesk_demo --w 1280 --h 720 --bitrate 6000000 --maxfps 60
```

Client side should read 12-byte video header followed by repeated 12+N packet frames as per the protocol above.



./start.sh --base-port 27183 --socket mirrordesk --w 1280 --h 720 --bitrate 6000000 --maxfps 60