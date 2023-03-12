cmake_minimum_required(VERSION 3.21.0)

project(rps VERSION 1.0)

# Set the C++ standard to C++20
set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the protobuf package
find_package(Protobuf REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread)
# Find GTest library and headers
find_package(GTest REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})
add_subdirectory(gtest)

# Specify the proto file
set(PROTO_FILE_GAME_SESSION proto/gameSession.proto)

# Generate the C++ code from the proto file
protobuf_generate_cpp(PROTO_SRCS_GAME_SESSION PROTO_HDRS_GAME_SESSION ${PROTO_FILE_GAME_SESSION})
protobuf_generate_python(PY_SOURCES_GAME_SESSION ${PROTO_FILE_GAME_SESSION})

# Add your target executable
add_executable(rps rps.cpp                                                                                                  /
        ${MY_SOURCES}                                                                                                       /
        ${PROTO_SRCS_GAME_SESSION}                                                                                          /
        ${PROTO_HDRS_GAME_SESSION}                                                                                          /         /
        ${PY_SOURCES_GAME_SESSION}                                                                                          /         /
    src/websocketListener.h src/websocketSession.h src/gameSessionManager.h src/gameSession.h util/util.h)

add_executable(tests src/gameSessionManager.h
        ${MY_SOURCES}                                                                                                       /
        ${PROTO_SRCS_GAME_SESSION}                                                                                          /
        ${PROTO_HDRS_GAME_SESSION}                                                                                          /
        test/gtestGameSessionManager.cpp
        src/gameSession.h
        util/util.h)

# Link
target_link_libraries(rps protobuf::libprotobuf ${Boost_LIBRARIES})
target_link_libraries(tests protobuf::libprotobuf GTest::GTest GTest::Main)

add_test(NAME GtestGameManager COMMAND tests)




