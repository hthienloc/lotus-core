cmake_minimum_required(VERSION 3.10)
project(lotus_engine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Includes
include_directories(include)

# Core Library (Static)
add_library(lotus_engine_core STATIC
    src/core/syllable.cpp
    src/core/validator.cpp
    src/core/parser/parser.cpp
    src/core/parser/initial_parser.cpp
    src/core/parser/glide_parser.cpp
    src/core/parser/nucleus_parser.cpp
    src/core/parser/coda_parser.cpp
    src/core/engine.cpp
    src/features/shortcut_manager.cpp
    src/features/smart_typing.cpp
    src/linguistics/linguistics.cpp
    src/core/log.cpp
    src/api/capi.cpp
)
set_target_properties(lotus_engine_core PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Core Library (Shared)
add_library(lotus_engine_core_shared SHARED
    src/core/syllable.cpp
    src/core/validator.cpp
    src/core/parser/parser.cpp
    src/core/parser/initial_parser.cpp
    src/core/parser/glide_parser.cpp
    src/core/parser/nucleus_parser.cpp
    src/core/parser/coda_parser.cpp
    src/core/engine.cpp
    src/features/shortcut_manager.cpp
    src/features/smart_typing.cpp
    src/linguistics/linguistics.cpp
    src/core/log.cpp
    src/api/capi.cpp
)
set_target_properties(lotus_engine_core_shared PROPERTIES OUTPUT_NAME lotus_engine_core)

# Target Include Directories
target_include_directories(lotus_engine_core PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>)
target_include_directories(lotus_engine_core_shared PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>)

# Test Executable
add_executable(lotus_tests
    tests/test_main.cpp
    tests/test_syllable.cpp
    tests/test_validator.cpp
    tests/test_edge_cases.cpp
    tests/test_parser.cpp
    tests/test_engine.cpp
    tests/test_vni.cpp
    tests/test_phonology.cpp
    tests/test_tone_style.cpp
    tests/test_capi.cpp
    tests/test_capi_integration.cpp
    tests/test_edge_cases.cpp
    tests/test_features_stub.cpp
)
target_link_libraries(lotus_tests lotus_engine_core)

# Helper to run tests
add_custom_target(run_tests
    COMMAND ./lotus_tests
    DEPENDS lotus_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# TUI Demo
add_executable(lotus_tui tests/tui_demo.cpp)
target_link_libraries(lotus_tui lotus_engine_core)

# Benchmark
add_executable(lotus_bench tests/bench_engine.cpp)
target_link_libraries(lotus_bench lotus_engine_core)

# Install Targets
install(TARGETS lotus_engine_core lotus_engine_core_shared
    EXPORT lotus_engine-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

# Install Headers
install(DIRECTORY include/ DESTINATION include)

# Export CMake Config
install(EXPORT lotus_engine-targets
    FILE lotus_engine-config.cmake
    DESTINATION lib/cmake/lotus_engine
)
