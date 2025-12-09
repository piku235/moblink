cmake_minimum_required(VERSION 3.28)
project(moblink VERSION 0.4.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
find_package(OpenSSL REQUIRED)
pkg_check_modules(MOSQUITTO REQUIRED IMPORTED_TARGET libmosquitto)

include(cmake/CPM.cmake)
CPMAddPackage("gh:piku235/mobgtw#main")
CPMAddPackage("gh:cameron314/concurrentqueue@1.0.4")

set(MOBILUS_CACERT_FILE "${mobgtw_SOURCE_DIR}/certs/mobilelabs_mobilus_ca.crt")
file(READ "${MOBILUS_CACERT_FILE}" MOBILUS_CACERT)

configure_file(
  src/mobilus_cert.h.in
  "${CMAKE_BINARY_DIR}/src/mobilus_cert.h"
  @ONLY
)

add_executable(moblink
  src/MqttMobilusGtwActor.cpp
  src/TargetMqttActor.cpp
  src/TargetMqttClient.cpp
  src/main.cpp
)

if(DEFINED MOBILUS_DSN)
  target_compile_definitions(moblink PRIVATE DEFAULT_MOBILUS_DSN=${MOBILUS_DSN})
endif()

target_include_directories(moblink
  PRIVATE
    include
    ${CMAKE_BINARY_DIR}/src
)

target_link_libraries(moblink
  PRIVATE
    mobgtw
    concurrentqueue
    OpenSSL::Crypto
    PkgConfig::MOSQUITTO
)

set(CPACK_GENERATOR "DEB;TGZ")

set(CPACK_PACKAGE_NAME "moblink")
set(CPACK_PACKAGE_VENDOR "Jungi")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CMAKE_SYSTEM_NAME}")
set(CPACK_PACKAGE_VERSION "${CMAKE_PROJECT_VERSION}")
set(CPACK_PACKAGE_CONTACT "Piotr Kugla <piku235@gmail.com>")
set(CPACK_PACKAGE_DESCRIPTION "A lightweight bridge that links the Mobilus Cosmo GTW and a target MQTT broker")

set(CPACK_DEBIAN_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}.deb")
set(CPACK_DEBIAN_PACKAGE_SECTION "net")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libmosquitto1, libssl3")
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/packaging/deb/postinst;${CMAKE_CURRENT_SOURCE_DIR}/packaging/deb/prerm;${CMAKE_CURRENT_SOURCE_DIR}/packaging/deb/conffiles")

install(TARGETS moblink RUNTIME DESTINATION bin)
install(FILES packaging/common/moblink.conf DESTINATION /etc)
install(FILES packaging/common/moblink.service DESTINATION /lib/systemd/system)

set(CPACK_STRIP_FILES TRUE)
set(CPACK_SET_DESTDIR ON)

include(CPack)
