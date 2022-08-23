add_executable(walkaroundmp_server server.cpp NetStuff.cpp NetStuff.h)
target_link_libraries(walkaroundmp_server PUBLIC
        GameNetworkingSockets::GameNetworkingSockets
        walkaroundmp_shared)
target_include_directories(walkaroundmp_server PUBLIC
        GameNetworkingSockets::GameNetworkingSockets
        walkaroundmp_shared)
