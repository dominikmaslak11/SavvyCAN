# ═══════════════════════════════════════════════════════════════════════════
# SavvyCAN Android Build Configuration
# Build with: cmake -B build_android -S android \
#   -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
#   -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=26 -GNinja \
#   -DCMAKE_PREFIX_PATH=$QT_ANDROID
# ═══════════════════════════════════════════════════════════════════════════

cmake_minimum_required(VERSION 3.21)

project(SavvyCAN-Android
    VERSION 223.0
    DESCRIPTION "SavvyCAN CAN bus analyzer — full Android port"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/../ui")

# ── Qt for Android modules ────────────────────────────────────────────────
find_package(Qt6 6.5 REQUIRED COMPONENTS
    Core Gui Widgets PrintSupport Qml Quick QuickControls2 QuickLayouts
    Network Svg Help OpenGL OpenGLWidgets
)

add_compile_definitions(
    QCUSTOMPLOT_USE_OPENGL
    ANDROID_PLATFORM
    QT_NO_SERIALBUS
)

# ── RESOURCES ─────────────────────────────────────────────────────────────
set(RESOURCES android.qrc)

# ── SOURCES — all SavvyCAN core files ─────────────────────────────────────
set(SOURCES
    # Android entry point & managers
    ../src/android/main_android.cpp
    ../src/android/BluetoothManager.cpp
    ../src/android/BleManager.cpp
    ../src/android/UsbHostManager.cpp
    ../src/android/WifiManager.cpp
    ../src/android/FrameListModel.cpp

    # Core
    ../src/core/can_structs.cpp
    ../src/core/utility.cpp
    ../src/core/simplecrypt.cpp
    ../src/core/futuristic_theme.cpp

    # Models
    ../src/models/framestore.cpp
    ../src/models/canframemodel.cpp
    ../src/models/canfilter.cpp
    ../src/models/filterutility.cpp

    # I/O
    ../src/io/framefileio.cpp
    ../src/io/blfhandler.cpp
    ../src/io/pcaplite.cpp

    # DBC
    ../dbc/dbc_classes.cpp
    ../dbc/dbchandler.cpp

    # Bus protocols
    ../bus_protocols/isotp_handler.cpp
    ../bus_protocols/j1939_handler.cpp
    ../bus_protocols/uds_handler.cpp

    # Mock QtSerialBus implementation (replaces desktop QtSerialBus)
    mock_serialbus/qcanbusframe.cpp
    mock_serialbus/qcanbusdevice.cpp
    mock_serialbus/qserialport.cpp

    # Stub MOC sources (required for inline Q_OBJECT classes in stubs)
    mock_serialbus/mainwindow.h
    mock_serialbus/connections/canconmanager.h
)

set(HEADERS
    ../src/android/BluetoothManager.h
    ../src/android/BleManager.h
    ../src/android/UsbHostManager.h
    ../src/android/WifiManager.h
    ../src/android/FrameListModel.h

    ../src/core/can_structs.h
    ../src/core/can_trigger_structs.h
    ../src/core/config.h
    ../src/core/simplecrypt.h
    ../src/core/utility.h
    ../src/core/futuristic_theme.h

    ../src/models/canframemodel.h
    ../src/models/framestore.h
    ../src/models/canfilter.h
    ../src/models/filterutility.h

    ../src/io/framefileio.h
    ../src/io/blfhandler.h
    ../src/io/pcaplite.h

    ../dbc/dbc_classes.h
    ../dbc/dbchandler.h

    ../bus_protocols/isotp_handler.h
    ../bus_protocols/isotp_message.h
    ../bus_protocols/j1939_handler.h
    ../bus_protocols/uds_handler.h

    mock_serialbus/qcanbusframe.h
    mock_serialbus/qcanbus.h
    mock_serialbus/qcanbusdevice.h
    mock_serialbus/qserialport.h
    mock_serialbus/mock_httpserver.h
    mock_serialbus/mock_websockets.h
    mock_serialbus/mock_python.h

    ../utils/lfqueue.h
)

# ── Executable ────────────────────────────────────────────────────────────
qt_add_executable(SavvyCAN-Android ${SOURCES} ${HEADERS} ${RESOURCES})

target_link_libraries(SavvyCAN-Android PRIVATE
    Qt6::Core Qt6::Gui Qt6::Widgets Qt6::PrintSupport
    Qt6::Qml Qt6::Quick Qt6::QuickControls2 Qt6::QuickLayouts
    Qt6::Network Qt6::Svg Qt6::Help Qt6::OpenGL Qt6::OpenGLWidgets
)

target_include_directories(SavvyCAN-Android PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/mock_serialbus
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/models
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/io
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/android
    ${CMAKE_CURRENT_SOURCE_DIR}/../connections
    ${CMAKE_CURRENT_SOURCE_DIR}/../dbc
    ${CMAKE_CURRENT_SOURCE_DIR}/../bus_protocols
    ${CMAKE_CURRENT_SOURCE_DIR}/../mqtt
    ${CMAKE_CURRENT_SOURCE_DIR}/../re
    ${CMAKE_CURRENT_SOURCE_DIR}/../re/sniffer
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils
)

# ── QML module ────────────────────────────────────────────────────────────
qt_add_qml_module(SavvyCAN-Android
    URI "com.savvycan.android"
    VERSION 1.0
    QML_FILES
        qml/MainView.qml
        qml/ConnectionPage.qml
        qml/FrameListPage.qml
        qml/FileIOPage.qml
        qml/GraphPage.qml
        qml/SnifferPage.qml
        qml/SignalViewerPage.qml
        qml/UDSScannerPage.qml
        qml/ISOTPPage.qml
        qml/FuzzingPage.qml
        qml/FrameSenderPage.qml
        qml/PlaybackPage.qml
        qml/CANBridgePage.qml
        qml/DBCEditorPage.qml
        qml/FileComparePage.qml
        qml/SettingsPage.qml
)

# ── Android APK package ───────────────────────────────────────────────────
set_target_properties(SavvyCAN-Android PROPERTIES
    QT_ANDROID_PACKAGE_NAME "com.savvycan.android"
    QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
    QT_ANDROID_VERSION_CODE 1
    QT_ANDROID_VERSION_NAME "1.0.0"
    QT_ANDROID_MIN_SDK_VERSION 26
    QT_ANDROID_TARGET_SDK_VERSION 34
)