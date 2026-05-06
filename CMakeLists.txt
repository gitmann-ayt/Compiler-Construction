cmake_minimum_required(VERSION 3.20)

project(cc_mini_compiler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(CC_USE_FLEX_BISON "Use Flex/Bison generated lexer/parser (requires flex/bison or win_flex/win_bison)" OFF)

add_library(cc_core
    src/ast.cpp
    src/lexer.cpp
    src/parser.cpp
    src/semantics.cpp
    src/tac.cpp
    src/optimizer.cpp
    src/tac_interpreter.cpp
    src/c_emitter.cpp
    src/util.cpp
)

target_include_directories(cc_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(compiler src/main.cpp)

target_link_libraries(compiler PRIVATE cc_core)

add_executable(ll1_tool
    tools/ll1_main.cpp
    tools/ll1.cpp
    src/util.cpp
)

target_include_directories(ll1_tool PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tools ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(postfix_eval tools/postfix_eval.cpp)

target_include_directories(postfix_eval PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

if (CC_USE_FLEX_BISON)
    message(STATUS "CC_USE_FLEX_BISON is ON. You must have flex/bison (or win_flex/win_bison) in PATH.")
    # The repository includes .l/.y sources under grammar/ for coursework deliverables.
    # This CMake project defaults to the hand-written frontend so it builds out-of-the-box.
endif()
