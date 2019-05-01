
set(sources
    Address.h
    Client.cpp
    Client.h
    Connection.h
    DirectLine.cpp
    DirectLine.h
    Hostname.cpp
    Hostname.h
    Listener.cpp
    Listener.h
    Main.cpp
    ProxyLine.cpp
    ProxyLine.h
)

add_executable(dehprox
    ${sources}
)

target_link_libraries(dehprox PRIVATE
    libnstd
)

source_group("" FILES ${sources})
