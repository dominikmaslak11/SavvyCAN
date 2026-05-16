# ═══════════════════════════════════════════════════════════════════════════
# SavvyCAN Android Build Configuration
# Build with: cmake -B build_android -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
#             -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=26 -GNinja
# ═══════════════════════════════════════════════════════════════════════════

cmake_minimum_required(VERSION 3.21)

project(SavvyCAN-Android
    VERSION 223.0
    DESCRIPTION "SavvyCAN CAN bus analyzer for Android"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ── Qt for Android modules ────────────────────────────────────────────────
find_package(Qt6 6.5 REQUIRED COMPONENTS
    Core Gui Widgets Qml Quick QuickControls2
    SerialBus SerialPort
    Bluetooth          # ← Bluetooth for ELM327 / ESP32
    AndroidExtras      # ← Android USB Host for PCAN OTG
)

add_compile_definitions(QCUSTOMPLOT_USE_OPENGL ANDROID_PLATFORM)

# ── Sources (subset — mobile-optimized) ───────────────────────────────────
set(SOURCES
    ../src/android/main_android.cpp
    ../src/android/BluetoothManager.cpp
    ../src/android/UsbHostManager.cpp

    # Core (reused from desktop)
    ../src/core/can_structs.cpp
    ../src/core/utility.cpp
    ../src/core/futuristic_theme.cpp

    # Models
    ../src/models/framestore.cpp
    ../src/models/canframemodel.cpp
    ../src/models/canfilter.cpp
    ../src/models/filterutility.cpp

    # IO
    ../src/io/framefileio.cpp
    ../src/io/blfhandler.cpp

    # DBC
    ../dbc/dbc_classes.cpp
    ../dbc/dbchandler.cpp
)

set(HEADERS
    ../src/android/BluetoothManager.h
    ../src/android/UsbHostManager.h
)

set(RESOURCES
    android/android.qrc
)

# ── QML resources for mobile UI ───────────────────────────────────────────
qt_add_qml_module(SavvyCAN-Android
    URI "com.savvycan.android"
    VERSION 1.0
    QML_FILES
        qml/MainView.qml
)

# ── Executable ────────────────────────────────────────────────────────────
qt_add_executable(SavvyCAN-Android ${SOURCES} ${HEADERS} ${RESOURCES})

target_link_libraries(SavvyCAN-Android PRIVATE
    Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Qml Qt6::Quick Qt6::QuickControls2
    Qt6::SerialBus Qt6::SerialPort
    Qt6::Bluetooth
    Qt6::AndroidExtras
)

target_include_directories(SavvyCAN-Android PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/models
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/io
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/android
    ${CMAKE_CURRENT_SOURCE_DIR}/../connections
    ${CMAKE_CURRENT_SOURCE_DIR}/../dbc
)

# ── Android APK package ───────────────────────────────────────────────────
set_target_properties(SavvyCAN-Android PROPERTIES
    QT_ANDROID_PACKAGE_NAME "com.savvycan.android"
    QT_ANDROID_VERSION_CODE 1
    QT_ANDROID_VERSION_NAME "1.0.0"
    QT_ANDROID_MIN_SDK_VERSION 26
    QT_ANDROID_TARGET_SDK_VERSION 34
)
