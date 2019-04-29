
add_executable(dehprox
    Main.cpp
)

target_link_libraries(dehprox PRIVATE
    libnstd
)
