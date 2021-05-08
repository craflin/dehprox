

set(DISABLE_ADD_TEST True)
function(add_test)
    if(NOT DISABLE_ADD_TEST)
        _add_test(${ARGV})
    endif()
endfunction()

set(DISABLE_INSTALL True)
function(install)
    if(NOT DISABLE_INSTALL)
    _install(${ARGV})
    endif()
endfunction()

set(DISABLE_ENABLE_TESTING True)
function(enable_testing)
    if(NOT DISABLE_ENABLE_TESTING)
        _enable_testing(${ARGV})
    endif()
endfunction()

set(DISABLE_ADD_SUBDIRECTORY True)
function(add_subdirectory)
    if(NOT DISABLE_ADD_SUBDIRECTORY OR "${ARGV0}" STREQUAL "libnstd" OR "${ARGV0}" STREQUAL "src" OR "${ARGV0}" STREQUAL "Socket" OR "${ARGV0}" STREQUAL "Document")
        _add_subdirectory(${ARGV})
    endif()
endfunction()

add_subdirectory(libnstd)

set_property(TARGET nstd nstdSocket nstdDocument PROPERTY FOLDER "3rdparty")
