cmake_minimum_required(VERSION 3.21.0)

project(rps VERSION 1.0)

# Set the C++ standard to C++20
set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the protobuf package
find_package(Protobuf REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${OPENSSL_INCLUDE_DIR})

# Specify the proto file
set(PROTO_FILE_GAME_SESSION proto/gameSession.proto)
SET(PROTO_FILE_GAME_PLAY proto/gamePlay.proto)

# Generate the C++ code from the proto file
protobuf_generate_cpp(PROTO_SRCS_GAME_SESSION PROTO_HDRS_GAME_SESSION ${PROTO_FILE_GAME_SESSION})
protobuf_generate_cpp(PROTO_SRCS_GAME_PLAY PROTO_HDRS_GAME_PLAY ${PROTO_FILE_GAME_PLAY})

protobuf_generate_python(PY_SOURCES_GAME_SESSION ${PROTO_FILE_GAME_SESSION})
protobuf_generate_python(PY_SOURCES_GAME_PLAY ${PROTO_FILE_GAME_PLAY})

# Add your target executable
add_executable(rps rps.cpp          /
        ${MY_SOURCES}               /
        ${PROTO_SRCS_GAME_SESSION}  /
        ${PROTO_HDRS_GAME_SESSION}  /
        ${PROTO_SRCS_GAME_PLAY}     /
        ${PROTO_HDRS_GAME_PLAY}     /
        ${PY_SOURCES_GAME_SESSION}  /
        ${PY_SOURCES_GAME_PLAY}     /
        src/websocketListener.h /
        src/websocketSession.h /
        src/gameSessionManager.h /
        src/gameSession.h /
        util/util.h /
)

# Link the protobuf library
target_link_libraries(rps protobuf::libprotobuf ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES})




