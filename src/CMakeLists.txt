
set(sources
    Main.cpp
    Client.cpp
    Client.h
    Connection.h
    ServerHandler.cpp
    ServerHandler.h
    Uplink.cpp
    Uplink.h
)

add_executable(dehprox
    ${sources}
)

target_link_libraries(dehprox PRIVATE
    libnstd
)

source_group("" FILES ${sources})
