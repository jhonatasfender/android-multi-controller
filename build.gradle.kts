plugins {
    alias(libs.plugins.composeHotReload) apply false
    alias(libs.plugins.composeMultiplatform) apply false
    alias(libs.plugins.composeCompiler) apply false
    alias(libs.plugins.kotlinMultiplatform) apply false
    id("com.android.application") version "8.5.2" apply false
    id("org.jetbrains.kotlin.android") version "2.2.0" apply false
}

val adbPath = providers.environmentVariable("ADB").orElse("adb")
val serialProp = providers.gradleProperty("serial").orNull
val socketProp = providers.gradleProperty("socket").orElse("mirrordesk_demo")
val portProp = providers.gradleProperty("port").orElse("27183")
val wProp = providers.gradleProperty("w").orElse("1280")
val hProp = providers.gradleProperty("h").orElse("720")
val bitrateProp = providers.gradleProperty("bitrate").orElse("6000000")
val maxfpsProp = providers.gradleProperty("maxfps").orElse("60")

val exportServerJar = tasks.register("exportAndroidServerJarProxy") { dependsOn(":androidServer:exportServerJar") }

val pushAndroidServerJar = tasks.register<Exec>("adbPushAndroidServerJar") {
    dependsOn(exportServerJar)
    group = "mirrordesk"
    val jarFile = project(":androidServer").layout.buildDirectory.file("dist/mirrordesk-android-server.jar")
    val serialArg = serialProp?.let { "-s $it" } ?: ""
    commandLine("sh", "-lc", "${adbPath.get()} ${serialArg} push ${jarFile.get().asFile.absolutePath} /data/local/tmp/mirrordesk-android-server.jar")
}

val adbForward = tasks.register<Exec>("adbForwardAndroidServer") {
    dependsOn(pushAndroidServerJar)
    group = "mirrordesk"
    val serialArg = serialProp?.let { "-s $it" } ?: ""
    val socketName = socketProp.get()
    val port = portProp.get()
    commandLine(
        "sh", "-lc",
        "${adbPath.get()} ${serialArg} forward --remove tcp:${port} >/dev/null 2>&1 || true; " +
                "${adbPath.get()} ${serialArg} forward tcp:${port} localabstract:${'$'}{socketName}"
    )
}

val startAndroidServer = tasks.register<Exec>("adbStartAndroidServer") {
    dependsOn(adbForward)
    group = "mirrordesk"
    val serialArg = serialProp?.let { "-s $it" } ?: ""
    val socketName = socketProp.get()
    val w = wProp.get()
    val h = hProp.get()
    val bitrate = bitrateProp.get()
    val maxfps = maxfpsProp.get()
    val startCmd = "CLASSPATH=/data/local/tmp/mirrordesk-android-server.jar app_process / com.mirrordesk.androidserver.MiniServer --socket ${socketName} --w ${w} --h ${h} --bitrate ${bitrate} --maxfps ${maxfps}"
    commandLine(
        "sh", "-lc",
        "${adbPath.get()} ${serialArg} shell nohup sh -c '${startCmd} </dev/null >/dev/null 2>&1 &'"
    )
}

tasks.register("runDesktopWithAndroidServer") {
    group = "mirrordesk"
    dependsOn(startAndroidServer)
    dependsOn(project(":composeApp").tasks.named("run"))
    project(":composeApp").tasks.named("run").configure { this.mustRunAfter(startAndroidServer) }
}