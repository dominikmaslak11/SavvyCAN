cmake_minimum_required(VERSION 3.21)

# ═══════════════════════════════════════════════════════════════════════════
# SavvyCAN — CMake build (modernized from qmake, 2026-05-15)
# Requires Qt 6.2+ (tested on 6.11)
# ═══════════════════════════════════════════════════════════════════════════

project(SavvyCAN
    VERSION 223.0
    DESCRIPTION "Qt6 cross-platform CAN bus analysis tool"
    HOMEPAGE_URL "https://github.com/collin80/SavvyCAN"
    LANGUAGES CXX
)

# ── C++ standard ──────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/ui")

# ── Qt6 ───────────────────────────────────────────────────────────────────
find_package(Qt6 6.2 REQUIRED COMPONENTS
    Core Gui Widgets PrintSupport Qml SerialBus SerialPort Help Network OpenGL OpenGLWidgets HttpServer WebSockets
)

# ── Compile definitions ───────────────────────────────────────────────────
add_compile_definitions(QCUSTOMPLOT_USE_OPENGL)

# ── Python / pybind11 ─────────────────────────────────────────────────────
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
find_package(pybind11 REQUIRED)
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_definitions(QT_NO_DEBUG_OUTPUT)
endif()

# ── Sources — organized by module ─────────────────────────────────────────
set(SOURCES
    src/app/main.cpp
    src/app/mainwindow.cpp

    src/core/can_structs.cpp
    src/core/simplecrypt.cpp
    src/core/utility.cpp
    src/core/futuristic_theme.cpp

    src/models/canframemodel.cpp
    src/models/framestore.cpp
    src/models/canfilter.cpp
    src/models/filterutility.cpp

    src/views/bisectwindow.cpp
    src/views/candatagrid.cpp
    src/views/canbridgewindow.cpp
    src/views/frameplaybackwindow.cpp
    src/views/frameplaybackobject.cpp
    src/views/framesenderwindow.cpp
    src/views/framesenderobject.cpp
    src/views/helpwindow.cpp
    src/views/signalviewerwindow.cpp
    src/views/jsedit.cpp
    src/views/triggerdialog.cpp
    src/views/mainsettingsdialog.cpp
    src/views/motorcontrollerconfigwindow.cpp
    src/views/scriptingwindow.cpp
    src/views/scriptcontainer.cpp
    src/views/sidebarwidget.cpp
    src/views/windowregistry.cpp

    src/io/framefileio.cpp
    src/io/blfhandler.cpp
    src/io/pcaplite.cpp

    pythonbridge.cpp
    pythonconsole.cpp
    restapiserver.cpp
    pluginmanager.cpp

    qcpaxistickerhex.cpp
    qcustomplot.cpp

    bus_protocols/isotp_handler.cpp
    bus_protocols/j1939_handler.cpp
    bus_protocols/uds_handler.cpp

    connections/canbus.cpp
    connections/canconnection.cpp
    connections/canconnectionmodel.cpp
    connections/canconfactory.cpp
    connections/canconmanager.cpp
    connections/canlogserver.cpp
    connections/canserver.cpp
    connections/connectionwindow.cpp
    connections/gvretserial.cpp
    connections/lawicel_serial.cpp
    connections/mqtt_bus.cpp
    connections/newconnectiondialog.cpp
    connections/serialbusconnection.cpp
    connections/socketcand.cpp

    dbc/dbc_classes.cpp
    dbc/dbchandler.cpp
    dbc/dbcloadsavewindow.cpp
    dbc/dbcmaineditor.cpp
    dbc/dbcmessageeditor.cpp
    dbc/dbcnodeeditor.cpp
    dbc/dbcnodeduplicateeditor.cpp
    dbc/dbcnoderebaseeditor.cpp
    dbc/dbcsignaleditor.cpp

    mqtt/qmqtt_client.cpp
    mqtt/qmqtt_client_p.cpp
    mqtt/qmqtt_frame.cpp
    mqtt/qmqtt_message.cpp
    mqtt/qmqtt_network.cpp
    mqtt/qmqtt_router.cpp
    mqtt/qmqtt_routesubscription.cpp
    mqtt/qmqtt_socket.cpp
    mqtt/qmqtt_ssl_socket.cpp
    mqtt/qmqtt_timer.cpp
    mqtt/qmqtt_websocket.cpp
    mqtt/qmqtt_websocketiodevice.cpp

    re/dbccomparatorwindow.cpp
    re/discretestatewindow.cpp
    re/filecomparatorwindow.cpp
    re/flowviewwindow.cpp
    re/frameinfowindow.cpp
    re/fuzzingwindow.cpp
    re/graphingwindow.cpp
    re/isotp_interpreterwindow.cpp
    re/newgraphdialog.cpp
    re/rangestatewindow.cpp
    re/temporalgraphwindow.cpp
    re/udsfirmwareuploaderwindow.cpp
    re/udsscanwindow.cpp

    re/sniffer/snifferitem.cpp
    re/sniffer/sniffermodel.cpp
    re/sniffer/snifferwindow.cpp
    re/sniffer/SnifferDelegate.cpp
)

set(HEADERS
    src/app/mainwindow.h

    src/core/can_structs.h
    src/core/can_trigger_structs.h
    src/core/config.h
    src/core/simplecrypt.h
    src/core/utility.h
    src/core/futuristic_theme.h

    src/models/canframemodel.h
    src/models/framestore.h
    src/models/canfilter.h
    src/models/filterutility.h

    src/views/bisectwindow.h
    src/views/candatagrid.h
    src/views/canbridgewindow.h
    src/views/frameplaybackwindow.h
    src/views/frameplaybackobject.h
    src/views/framesenderwindow.h
    src/views/framesenderobject.h
    src/views/helpwindow.h
    src/views/signalviewerwindow.h
    src/views/jsedit.h
    src/views/triggerdialog.h
    src/views/mainsettingsdialog.h
    src/views/motorcontrollerconfigwindow.h
    src/views/scriptingwindow.h
    src/views/scriptcontainer.h
    src/views/sidebarwidget.h
    src/views/windowregistry.h

    src/io/framefileio.h
    src/io/blfhandler.h
    src/io/pcaplite.h

    pythonbridge.h
    pythonconsole.h
    restapiserver.h
    iplugin.h
    pluginmanager.h

    qcpaxistickerhex.h
    qcustomplot.h

    bus_protocols/isotp_handler.h
    bus_protocols/isotp_message.h
    bus_protocols/j1939_handler.h
    bus_protocols/uds_handler.h

    connections/canbus.h
    connections/canconnection.h
    connections/canconnectionmodel.h
    connections/canconconst.h
    connections/canconfactory.h
    connections/canconmanager.h
    connections/canlogserver.h
    connections/canserver.h
    connections/connectionwindow.h
    connections/gvretserial.h
    connections/lawicel_serial.h
    connections/mqtt_bus.h
    connections/newconnectiondialog.h
    connections/serialbusconnection.h
    connections/socketcand.h

    dbc/dbc_classes.h
    dbc/dbchandler.h
    dbc/dbcloadsavewindow.h
    dbc/dbcmaineditor.h
    dbc/dbcmessageeditor.h
    dbc/dbcnodeeditor.h
    dbc/dbcnodeduplicateeditor.h
    dbc/dbcnoderebaseeditor.h
    dbc/dbcsignaleditor.h

    mqtt/qmqtt.h
    mqtt/qmqtt_client.h
    mqtt/qmqtt_client_p.h
    mqtt/qmqtt_frame.h
    mqtt/qmqtt_global.h
    mqtt/qmqtt_message.h
    mqtt/qmqtt_message_p.h
    mqtt/qmqtt_network_p.h
    mqtt/qmqtt_networkinterface.h
    mqtt/qmqtt_routedmessage.h
    mqtt/qmqtt_router.h
    mqtt/qmqtt_routesubscription.h
    mqtt/qmqtt_socket_p.h
    mqtt/qmqtt_socketinterface.h
    mqtt/qmqtt_ssl_socket_p.h
    mqtt/qmqtt_timer_p.h
    mqtt/qmqtt_timerinterface.h
    mqtt/qmqtt_websocket_p.h
    mqtt/qmqtt_websocketiodevice_p.h

    re/dbccomparatorwindow.h
    re/discretestatewindow.h
    re/filecomparatorwindow.h
    re/flowviewwindow.h
    re/frameinfowindow.h
    re/fuzzingwindow.h
    re/graphingwindow.h
    re/isotp_interpreterwindow.h
    re/newgraphdialog.h
    re/rangestatewindow.h
    re/temporalgraphwindow.h
    re/udsfirmwareuploaderwindow.h
    re/udsscanwindow.h

    re/sniffer/SnifferDelegate.h
    re/sniffer/snifferitem.h
    re/sniffer/sniffermodel.h
    re/sniffer/snifferwindow.h

    utils/lfqueue.h
)

set(FORMS
    src/views/triggerdialog.ui
    ui/bisectwindow.ui
    ui/canbridgewindow.ui
    ui/candatagrid.ui
    ui/connectionwindow.ui
    ui/dbccomparatorwindow.ui
    ui/dbcloadsavewindow.ui
    ui/dbcmaineditor.ui
    ui/dbcmessageeditor.ui
    ui/dbcnodeeditor.ui
    ui/dbcnodeduplicateeditor.ui
    ui/dbcnoderebaseeditor.ui
    ui/dbcsignaleditor.ui
    ui/discretestatewindow.ui
    ui/filecomparatorwindow.ui
    ui/flowviewwindow.ui
    ui/frameinfowindow.ui
    ui/frameplaybackwindow.ui
    ui/framesenderwindow.ui
    ui/fuzzingwindow.ui
    ui/graphingwindow.ui
    ui/helpwindow.ui
    ui/isotp_interpreterwindow.ui
    ui/mainsettingsdialog.ui
    ui/mainwindow.ui
    ui/motorcontrollerconfigwindow.ui
    ui/newconnectiondialog.ui
    ui/newgraphdialog.ui
    ui/rangestatewindow.ui
    ui/scriptingwindow.ui
    ui/signalviewerwindow.ui
    ui/snifferwindow.ui
    ui/temporalgraphwindow.ui
    ui/udsfirmwareuploaderwindow.ui
    ui/udsscanwindow.ui
)

set(RESOURCES icons.qrc images.qrc)

# ── Executable ────────────────────────────────────────────────────────────
qt_add_executable(SavvyCAN ${SOURCES} ${HEADERS} ${FORMS} ${RESOURCES} Info.plist.template)

# ── Link Qt modules ───────────────────────────────────────────────────────
target_link_libraries(SavvyCAN PRIVATE
    Qt6::Core Qt6::Gui Qt6::Widgets Qt6::PrintSupport Qt6::Qml
    Qt6::SerialBus Qt6::SerialPort Qt6::Help Qt6::Network Qt6::OpenGL Qt6::OpenGLWidgets
    Qt6::HttpServer
    Qt6::WebSockets
    pybind11::embed
    Python3::Python
)

# ── Include paths (ALL subdirectories for backward-compat includes) ───────
target_include_directories(SavvyCAN PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/app
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/models
    ${CMAKE_CURRENT_SOURCE_DIR}/src/views
    ${CMAKE_CURRENT_SOURCE_DIR}/src/io
    ${CMAKE_CURRENT_SOURCE_DIR}/connections
    ${CMAKE_CURRENT_SOURCE_DIR}/dbc
    ${CMAKE_CURRENT_SOURCE_DIR}/bus_protocols
    ${CMAKE_CURRENT_SOURCE_DIR}/mqtt
    ${CMAKE_CURRENT_SOURCE_DIR}/re
    ${CMAKE_CURRENT_SOURCE_DIR}/re/sniffer
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
)

# ── Platform-specific ─────────────────────────────────────────────────────
if(WIN32)
    target_link_libraries(SavvyCAN PRIVATE opengl32)
    set_target_properties(SavvyCAN PROPERTIES WIN32_EXECUTABLE TRUE
        RC_ICONS "${CMAKE_CURRENT_SOURCE_DIR}/icons/SavvyIcon.ico")
endif()

if(UNIX AND NOT APPLE)
    install(TARGETS SavvyCAN RUNTIME DESTINATION bin)
    install(FILES SavvyCAN.desktop DESTINATION share/applications)
endif()

if(APPLE)
    set_target_properties(SavvyCAN PROPERTIES MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.template"
        MACOSX_BUNDLE_ICON_FILE "SavvyIcon.icns")
endif()

install(DIRECTORY examples/  DESTINATION share/savvycan/examples)
install(DIRECTORY icons/    DESTINATION share/icons)
install(DIRECTORY help/     DESTINATION bin/help)

# ── Tests ─────────────────────────────────────────────────────────────────
option(BUILD_TESTS "Build unit tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
    message(STATUS "Tests enabled.")
endif()
