# Framestore unit tests
# Build with: cmake -DBUILD_TESTS=ON -B build_test && cmake --build build_test && cd build_test && ctest

find_package(Qt6 REQUIRED COMPONENTS Core Test)

enable_testing()

add_executable(test_framestore
    framestore_test.cpp
    ${CMAKE_SOURCE_DIR}/src/models/framestore.cpp
    ${CMAKE_SOURCE_DIR}/src/core/can_structs.cpp
)

target_include_directories(test_framestore PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/models
    ${CMAKE_SOURCE_DIR}/connections
)

target_link_libraries(test_framestore PRIVATE
    Qt6::Core
    Qt6::SerialBus
    Qt6::Test
)

add_test(NAME FramestoreTest COMMAND test_framestore)
