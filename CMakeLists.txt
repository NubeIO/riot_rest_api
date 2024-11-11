cmake_minimum_required(VERSION 3.14)
project(crow_test)
set(CMAKE_CXX_STANDARD 17)
include(FetchContent)

find_package(CapnProto REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem)



FetchContent_Declare(
        Crow
        GIT_REPOSITORY https://github.com/CrowCpp/Crow
        GIT_TAG v1.0+5
)
FetchContent_MakeAvailable(Crow)

# Your executable
add_executable(crow_test
        main.cpp
        schemas/package.capnp.c++
        ${CAPNP_SRCS}    # Add generated .cpp files
        ${CAPNP_HDRS}    # Add generated .h files
)

target_include_directories(crow_test PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}  # For generated Cap'n Proto headers
)

target_link_libraries(crow_test PRIVATE
        CapnProto::capnp
        CapnProto::capnp-rpc
        CapnProto::kj-async
        Boost::system
        Boost::filesystem
        Crow::Crow
)