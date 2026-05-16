# SavvyCAN Android Build Guide

## Wymagania

1. **Android Studio** (lub Android SDK CLI)
   - SDK Platform 34
   - NDK 26.1.10909125
   - Build Tools 34.0.0

2. **Qt 6.8 for Android**
   - Pobierz z https://www.qt.io/download
   - Komponenty: `android_arm64_v8a`
   - Moduły: QtCharts, QtSerialBus, QtSerialPort, QtBluetooth, QtAndroidExtras

3. **Java JDK 17**

## Budowanie (Windows)

```powershell
# Ustaw zmienne
$env:ANDROID_NDK = "C:\Users\...\AppData\Local\Android\Sdk\ndk\26.1.10909125"
$env:QT_ANDROID = "C:\Qt\6.8.2\android_arm64_v8a"
$env:JAVA_HOME = "C:\Program Files\Java\jdk-17"

# Konfiguracja
cmake -B build_android -S android `
  -DCMAKE_TOOLCHAIN_FILE="$env:ANDROID_NDK/build/cmake/android.toolchain.cmake" `
  -DANDROID_ABI=arm64-v8a `
  -DANDROID_PLATFORM=android-26 `
  -DCMAKE_BUILD_TYPE=Release `
  -GNinja `
  -DCMAKE_PREFIX_PATH="$env:QT_ANDROID"

# Budowanie APK
cmake --build build_android --target apk
```

Wynik: `build_android/android-build/SavvyCAN-Android.apk`

## Funkcje

- **Bluetooth**: ELM327 (OBD2), ESP32 CAN, adapters SPP
- **USB OTG**: PEAK PCAN-USB, PCAN-USB FD/Pro
- **QML UI**: Material Design (ciemny theme), lista ramek, quick send
- **Min. Android**: 8.0 (API 26)
