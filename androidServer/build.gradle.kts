plugins {
    id("com.android.application")
    id("org.jetbrains.kotlin.android")
}

android {
    namespace = "com.mirrordesk.androidserver"
    compileSdk = 34

    defaultConfig {
        applicationId = "com.mirrordesk.androidserver"
        minSdk = 24
        targetSdk = 34
        versionCode = 1
        versionName = "0.1"
    }

    buildTypes {
        release {
            isMinifyEnabled = false
        }
        debug {
            isMinifyEnabled = false
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }
    kotlinOptions {
        jvmTarget = "17"
    }
}

kotlin {
    jvmToolchain(17)
}

dependencies {
}

abstract class ExportServerJarTask : DefaultTask() {
    @get:InputDirectory
    abstract val apkDir: DirectoryProperty

    @get:OutputFile
    abstract val outFile: RegularFileProperty

    @TaskAction
    fun run() {
        val dir = apkDir.get().asFile
        val candidates = dir.listFiles { f -> f.isFile && f.name.endsWith(".apk") }?.toList().orEmpty()
        require(candidates.isNotEmpty()) { "No APK files found in ${dir.absolutePath}" }
        val apk = candidates.maxByOrNull { it.lastModified() }!!
        val out = outFile.get().asFile
        out.parentFile.mkdirs()
        apk.copyTo(out, overwrite = true)
    }
}

tasks.register<ExportServerJarTask>("exportServerJar") {
    group = "distribution"
    dependsOn("assembleRelease")
    val apkPath = layout.buildDirectory.dir("outputs/apk/release")
    apkDir.set(apkPath)
    outFile.set(layout.buildDirectory.file("dist/mirrordesk-android-server.jar"))
}


