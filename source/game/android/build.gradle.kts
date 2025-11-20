plugins {
    alias(libs.plugins.android.application)
}

val targetName = (project.findProperty("targetName") as String?) ?: "Lucaria"
val gamePackage = "com.lucaria.lib$targetName"

android {
    namespace = gamePackage
    compileSdk = 35

    defaultConfig {
        applicationId = gamePackage
        minSdk = 24
        targetSdk = 35
        versionCode = 1
        versionName = "1.0"
        manifestPlaceholders["NATIVE_TARGET_NAME"] = targetName
    }

    buildTypes {
        release {
            isMinifyEnabled = false
        }
        debug {
            // debuggable by default
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_11
        targetCompatibility = JavaVersion.VERSION_11
    }
}

dependencies {
    // You can actually remove everything,
    // but leaving appcompat/material out is good to avoid useless deps.
}
