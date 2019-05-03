
set(sources
    Address.h
    Client.cpp
    Client.h
    Connection.h
    DirectLine.cpp
    DirectLine.h
    DnsServer.cpp
    DnsServer.h
    Hostname.cpp
    Hostname.h
    Main.cpp
    ProxyLine.cpp
    ProxyLine.h
    ProxyServer.cpp
    ProxyServer.h
)

add_executable(dehprox
    ${sources}
)

target_link_libraries(dehprox PRIVATE
    libnstd
)

source_group("" FILES ${sources})
