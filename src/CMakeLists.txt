
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
    libnstd::Core libnstd::Socket
)

source_group("" FILES ${sources})

set_property(TARGET dehprox PROPERTY FOLDER "src")

install(TARGETS dehprox DESTINATION usr/sbin)
