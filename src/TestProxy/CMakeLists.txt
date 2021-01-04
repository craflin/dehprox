
set(sources
    Main.cpp
)

add_executable(TestProxy
    ${sources}
)

target_link_libraries(TestProxy PRIVATE
    libnstd::Core libnstd::Socket
)

source_group("" FILES ${sources})

set_property(TARGET TestProxy PROPERTY FOLDER "")
