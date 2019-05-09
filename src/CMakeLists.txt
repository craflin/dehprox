
set(sources
    Address.h
    Client.cpp
    Client.h
    Connection.h
    DirectLine.cpp
    DirectLine.h
    DnsDatabase.cpp
    DnsDatabase.h
    DnsServer.cpp
    DnsServer.h
    Main.cpp
    ProxyLine.cpp
    ProxyLine.h
    ProxyServer.cpp
    ProxyServer.h
    Settings.cpp
    Settings.h
)

add_executable(dehprox
    ${sources}
)

target_link_libraries(dehprox PRIVATE
    libnstd
)

source_group("" FILES ${sources})

install(TARGETS dehprox DESTINATION usr/sbin)
