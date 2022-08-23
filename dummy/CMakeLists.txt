add_executable(dummy dummyclient.cpp)
target_link_libraries(dummy PUBLIC
        GameNetworkingSockets::GameNetworkingSockets)
target_include_directories(dummy PUBLIC
        GameNetworkingSockets::GameNetworkingSockets)
