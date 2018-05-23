cmake_minimum_required(VERSION 3.1)

project(gitlook LANGUAGES CXX)

include(ExternalProject)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core Quick REQUIRED)

ExternalProject_Add(yaml-cpp-build
    PREFIX          3rdparty
    GIT_REPOSITORY  "https://github.com/jbeder/yaml-cpp.git"
    GIT_TAG         "yaml-cpp-0.6.2"
    UPDATE_COMMAND  "" # Skip annoying updates for every build
    INSTALL_COMMAND ""
    )

SET(YAML_CPP_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/3rdparty/src/yaml-cpp/include)
SET(YAML_CPP_LIB_DIR     ${CMAKE_CURRENT_BINARY_DIR}/3rdparty/src/yaml-cpp-build-build)

add_library(yaml-cpp SHARED IMPORTED)
set_target_properties(yaml-cpp PROPERTIES
    IMPORTED_LOCATION
    ${YAML_CPP_LIB_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}yaml-cpp${CMAKE_STATIC_LIBRARY_SUFFIX})
add_dependencies(yaml-cpp yaml-cpp-build)

add_executable(${PROJECT_NAME}
    "main.cpp"
    "cpp/Commits.cpp"
    "cpp/Git.cpp"
    "cpp/Settings.cpp"
    "cpp/Projects.cpp"
    "qml.qrc")

include_directories(cpp ${YAML_CPP_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Quick git2 yaml-cpp)
