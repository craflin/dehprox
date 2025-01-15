
cmake_minimum_required(VERSION 3.5)
cmake_policy(SET CMP0048 NEW)

project(dehprox VERSION 0.1.3)

set(CDEPLOY_NO_DEBUG_BUILD True)
set(CDEPLOY_NO_COMPILER True)

include(CDeploy)

add_subdirectory(3rdparty)
add_subdirectory(src)

set(CPACK_PACKAGE_CONTACT "Colin Graf <colin.graf@sovereign-labs.com>")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Transparent DNS and TCP to HTTP proxy redirector")
set(CMAKE_PROJECT_HOMEPAGE_URL "https://github.com/craflin/dehprox")
set(CPACK_PACKAGING_INSTALL_PREFIX "/")

if(CDEPLOY_OS MATCHES "ubuntu.*" OR CDEPLOY_OS MATCHES "raspbian.*")
    set(CPACK_GENERATOR "DEB")
    #set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")
    set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "${CMAKE_PROJECT_HOMEPAGE_URL}")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.15)")
    set(CPACK_DEBIAN_PACKAGE_SECTION "net")
endif()

if(CDEPLOY_OS MATCHES "centos.*" OR CDEPLOY_OS MATCHES "rocky.*")
    set(CPACK_GENERATOR "RPM")
    set(CPACK_RPM_PACKAGE_LICENSE "Apache License")
    set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "/usr/sbin")
endif()


include(CPack)

install(FILES etc/dehprox.conf DESTINATION etc)
install(FILES etc/default/dehprox DESTINATION etc/default)
install(PROGRAMS etc/init.d/dehprox DESTINATION etc/init.d)
