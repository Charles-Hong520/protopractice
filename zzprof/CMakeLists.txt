cmake_minimum_required(VERSION 3.10)

project(ProfileProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED CONFIG)

include_directories(${PROTOBUF_INCLUDE_DIRS})

set(SOURCES
    main.cpp
    parser.cpp
    profile.pb.cc
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} protobuf::libprotobuf protobuf::libprotoc Threads::Threads)

