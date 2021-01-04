add_subdirectory(TestProxy)
return()

set(sources
    Address.hpp
    Client.cpp
    Client.hpp
    Connection.hpp
    DirectLine.cpp
    DirectLine.hpp
    DnsDatabase.cpp
    DnsDatabase.hpp
    DnsServer.cpp
    DnsServer.hpp
    Main.cpp
    ProxyLine.cpp
    ProxyLine.hpp
    ProxyServer.cpp
    ProxyServer.hpp
    Settings.cpp
    Settings.hpp
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

add_subdirectory(TestProxy)

