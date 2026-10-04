cmake_minimum_required(VERSION 3.15)

project(OC_lab1_3sem_Suboch)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Creator
    ${CMAKE_CURRENT_SOURCE_DIR}/Main
    ${CMAKE_CURRENT_SOURCE_DIR}/Reporter
)

add_executable(Creator Creator/Creator.cpp)
add_executable(Main Main/Main.cpp)
add_executable(Reporter Reporter/Reporter.cpp)

target_link_libraries(Main PRIVATE Kernel32)