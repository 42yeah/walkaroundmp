add_executable(dummy dummyclient.cpp)
target_link_libraries(dummy PUBLIC
        GameNetworkingSockets::GameNetworkingSockets
        walkaroundmp_shared)
target_include_directories(dummy PUBLIC
        GameNetworkingSockets::GameNetworkingSockets
        walkaroundmp_shared)
