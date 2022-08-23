cmake_minimum_required(VERSION 3.23)
project(walkaroundmp)

set(CMAKE_CXX_STANDARD 14)
find_package(SDL2 REQUIRED)
find_package(SDL2_Image REQUIRED)
find_package(GameNetworkingSockets REQUIRED)
find_package(Protobuf REQUIRED)
add_executable(walkaroundmp main.cpp Game.cpp World.cpp constants.h NetStuff.cpp NetStuff.h)
target_link_libraries(walkaroundmp PUBLIC
        SDL2::SDL2
        SDL2_image::SDL2_image
        GameNetworkingSockets::GameNetworkingSockets)
target_include_directories(walkaroundmp PUBLIC
        SDL2::SDL2
        SDL2_image::SDL2_image
        GameNetworkingSockets::GameNetworkingSockets)
if (CMAKE_BUILD_TYPE STREQUAL Debug)
    add_compile_definitions(TILESET_PREFIX="/Users/42yeah/Projets/Arts/kenney_tinydungeon")
endif()

# server
add_executable(walkaroundmp_server server.cpp World.cpp constants.h NetStuff.cpp NetStuff.h)
target_link_libraries(walkaroundmp_server PUBLIC
        GameNetworkingSockets::GameNetworkingSockets)
target_include_directories(walkaroundmp_server PUBLIC
        GameNetworkingSockets::GameNetworkingSockets)

# dummy
add_executable(dummy dummyclient.cpp)
target_link_libraries(dummy PUBLIC
        GameNetworkingSockets::GameNetworkingSockets)
target_include_directories(dummy PUBLIC
        GameNetworkingSockets::GameNetworkingSockets)
