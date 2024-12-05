cmake_minimum_required(VERSION 3.17)
project(ce-rest-api)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-E")

find_package(CapnProto REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem)

include(FetchContent)
FetchContent_Declare(
        Crow
        GIT_REPOSITORY https://github.com/CrowCpp/Crow
        GIT_TAG v1.0+5
)
FetchContent_MakeAvailable(Crow)

# Your executable
add_executable(ce-rest-api
        main.cpp
        schemas/package.capnp.c++
)

target_include_directories(ce-rest-api PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}  # For generated Cap'n Proto headers
)

target_link_libraries(ce-rest-api
	/usr/local/lib/libcapnp-json.a
      	/usr/local/lib/libcapnp-websocket.a
       	/usr/local/lib/libcapnpc.a
	/usr/local/lib/libcapnp-rpc.a
	/usr/local/lib/libcapnp.a
	/usr/local/lib/libkj-async.a
       	/usr/local/lib/libkj-test.a
	/usr/local/lib/libkj-http.a
	/usr/local/lib/libkj.a
        Boost::system
        Boost::filesystem
        Crow::Crow
)
