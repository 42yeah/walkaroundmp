add_executable(walkaroundmp main.cpp Game.cpp)
target_link_libraries(walkaroundmp PUBLIC
        SDL2::SDL2
        SDL2_image::SDL2_image
        GameNetworkingSockets::GameNetworkingSockets
        walkaroundmp_shared)
target_include_directories(walkaroundmp PUBLIC
        SDL2::SDL2
        SDL2_image::SDL2_image
        GameNetworkingSockets::GameNetworkingSockets
        walkaroundmp_shared)
if (CMAKE_BUILD_TYPE STREQUAL Debug)
    add_compile_definitions(TILESET_PREFIX="/Users/42yeah/Projets/Arts/kenney_tinydungeon")
endif()
