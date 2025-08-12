plugins {
    alias(libs.plugins.kotlinMultiplatform)
    application
}

repositories {
    mavenCentral()
}

kotlin { jvm() }

application {
    mainClass.set("com.mirrordesk.serverstub.MainKt")
}

tasks.register<Jar>("fatJar") {
    group = "build"
    archiveClassifier.set("all")
    duplicatesStrategy = DuplicatesStrategy.EXCLUDE
    manifest {
        attributes["Main-Class"] = "com.mirrordesk.serverstub.MainKt"
    }
    from(sourceSets.main.get().output)
    dependsOn(configurations.runtimeClasspath)
    from({
        configurations.runtimeClasspath.get().filter { it.name.endsWith(".jar") }.map { zipTree(it) }
    })
}

