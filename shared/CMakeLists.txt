add_library(walkaroundmp_shared World.cpp)
set_target_properties(walkaroundmp_shared PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR})
