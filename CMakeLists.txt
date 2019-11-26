cmake_minimum_required(VERSION 3.1)

project(brigitte LANGUAGES CXX)

include(ExternalProject)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core Quick REQUIRED)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

add_executable(${PROJECT_NAME}
    "main.cpp"
    "cpp/Commits.cpp"
    "cpp/Git.cpp"
    "cpp/Settings.cpp"
    "cpp/Projects.cpp"
    "qml.qrc")

include_directories(cpp ${YAML_CPP_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME}
  Qt5::Core Qt5::Quick git2
  CONAN_PKG::yaml-cpp)
