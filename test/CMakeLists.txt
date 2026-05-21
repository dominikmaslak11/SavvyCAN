# SavvyCAN Unit Tests
# Build with: cmake -DBUILD_TESTS=ON -B build_test && cmake --build build_test

find_package(Qt6 REQUIRED COMPONENTS Core Test SerialBus)

enable_testing()

# ── Common sources needed by most tests ────────────────────────────────────
set(CORE_SOURCES
    ${CMAKE_SOURCE_DIR}/src/models/framestore.cpp
    ${CMAKE_SOURCE_DIR}/src/core/can_structs.cpp
)

set(CORE_INCLUDES
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/models
    ${CMAKE_SOURCE_DIR}/src/io
    ${CMAKE_SOURCE_DIR}/connections
    ${CMAKE_SOURCE_DIR}/dbc
    ${CMAKE_SOURCE_DIR}/bus_protocols
    ${CMAKE_SOURCE_DIR}/re/sniffer
    ${CMAKE_SOURCE_DIR}/utils
    /usr/include/x86_64-linux-gnu/qt6/QtSerialBus
    /usr/include/x86_64-linux-gnu/qt6/QtSerialPort
)

# ── Test 1: CANStructs + FrameSendData ─────────────────────────────────────
add_executable(test_canstructs
    test_canstructs.cpp
    ${CORE_SOURCES}
)
target_include_directories(test_canstructs PRIVATE ${CORE_INCLUDES})
target_link_libraries(test_canstructs PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::SerialBus Qt6::Test)
add_test(NAME CANStructsTest COMMAND test_canstructs)

# ── Test 2: FrameStore ─────────────────────────────────────────────────────
add_executable(test_framestore
    framestore_test.cpp
    ${CORE_SOURCES}
)
target_include_directories(test_framestore PRIVATE ${CORE_INCLUDES})
target_link_libraries(test_framestore PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::SerialBus Qt6::Test)
add_test(NAME FrameStoreTest COMMAND test_framestore)

# ── Test 3: Lock-free queue ────────────────────────────────────────────────
add_executable(test_lfqueue
    tst_lfqueue.cpp
)
target_include_directories(test_lfqueue PRIVATE ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/utils)
target_link_libraries(test_lfqueue PRIVATE Qt6::Core Qt6::Test)
add_test(NAME LFQueueTest COMMAND test_lfqueue)

# ── Test 4: Frame File I/O (CRTD, CSV) ─────────────────────────────────────
add_executable(test_fileio
    test_fileio.cpp
    ${CMAKE_SOURCE_DIR}/src/io/framefileio.cpp
    ${CMAKE_SOURCE_DIR}/src/io/blfhandler.cpp
    ${CMAKE_SOURCE_DIR}/src/io/pcaplite.cpp
    ${CMAKE_SOURCE_DIR}/src/core/can_structs.cpp
    ${CMAKE_SOURCE_DIR}/src/core/utility.cpp
)
target_include_directories(test_fileio PRIVATE ${CORE_INCLUDES})
target_link_libraries(test_fileio PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::SerialBus Qt6::Test)
add_test(NAME FileIOTest COMMAND test_fileio)

# ── Test 5: DBC ────────────────────────────────────────────────────────────
add_executable(test_dbc
    test_dbc.cpp
    ${CMAKE_SOURCE_DIR}/dbc/dbc_classes.cpp
    ${CMAKE_SOURCE_DIR}/dbc/dbchandler.cpp
    ${CMAKE_SOURCE_DIR}/src/core/can_structs.cpp
    ${CMAKE_SOURCE_DIR}/src/core/utility.cpp
)
target_include_directories(test_dbc PRIVATE ${CORE_INCLUDES})
target_link_libraries(test_dbc PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::SerialBus Qt6::Test)
add_test(NAME DBCTest COMMAND test_dbc)
