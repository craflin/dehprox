
cmake_minimum_required(VERSION 3.1)
cmake_policy(SET CMP0048 NEW)

project(dehprox VERSION 0.1.0)

add_subdirectory(3rdparty)
add_subdirectory(src)


set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_CONTACT "Colin Graf <colin.graf@sovereign-labs.com>")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Transparent DNS and TCP to HTTP proxy redirector")
set(CPACK_PACKAGING_INSTALL_PREFIX "/")
set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/craflin/dehprox")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.15)")
set(CPACK_DEBIAN_PACKAGE_SECTION "net")

include(CPack)

install(FILES etc/dehprox.conf DESTINATION etc)
install(FILES etc/default/dehprox DESTINATION etc/default)
install(PROGRAMS etc/init.d/dehprox DESTINATION etc/init.d)
