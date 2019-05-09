
cmake_minimum_required(VERSION 3.1)
cmake_policy(SET CMP0048 NEW)

project(dehprox VERSION 0.1.0)

add_subdirectory(3rdparty)
add_subdirectory(src)


set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_CONTACT "Colin Graf")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Transparent DNS and TCP to HTTP proxy redirector")
set(CPACK_PACKAGING_INSTALL_PREFIX "/")

include(CPack)

install(FILES etc/dehprox.conf DESTINATION etc)
install(PROGRAMS etc/init.d/dehprox DESTINATION etc/init.d)
