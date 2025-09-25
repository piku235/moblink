cmake_minimum_required(VERSION 3.25)
project(moblink VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
find_package(OpenSSL REQUIRED)
pkg_check_modules(MOSQUITTO REQUIRED IMPORTED_TARGET libmosquitto)

include(FetchContent)
FetchContent_Declare(
  mobilus_gtw_client
  URL https://github.com/piku235/mobilus-gtw-client/archive/refs/tags/v0.4.1.zip
)
FetchContent_MakeAvailable(mobilus_gtw_client)

set(MOBILUS_CACERT_FILE "${mobilus_gtw_client_SOURCE_DIR}/certs/mobilelabs_mobilus_ca.crt")
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

target_include_directories(moblink
  PRIVATE
    include
    ${CMAKE_BINARY_DIR}/src
)

target_link_libraries(moblink
  PRIVATE
    mobilus_gtw_client
    OpenSSL::Crypto
    PkgConfig::MOSQUITTO
)

set(CPACK_GENERATOR "DEB")

set(CPACK_PACKAGE_NAME "moblink")
set(CPACK_PACKAGE_VENDOR "Jungi")
set(CPACK_PACKAGE_VERSION "${CMAKE_PROJECT_VERSION}")
set(CPACK_PACKAGE_CONTACT "Piotr Kugla <piku235@gmail.com>")
set(CPACK_PACKAGE_DESCRIPTION "A lightweight bridge that links the Mobilus Cosmo GTW and a target MQTT broker")

set(CPACK_DEBIAN_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}.deb")
set(CPACK_DEBIAN_PACKAGE_SECTION "net")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libmosquitto1, libssl3")
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/packaging/deb/postinst;${CMAKE_CURRENT_SOURCE_DIR}/packaging/deb/prerm;${CMAKE_CURRENT_SOURCE_DIR}/packaging/deb/conffiles")

set(CPACK_STRIP_FILES TRUE)

install(TARGETS moblink RUNTIME DESTINATION bin)
install(FILES packaging/deb/moblink.conf DESTINATION /etc)
install(FILES packaging/deb/moblink.service DESTINATION /lib/systemd/system)

include(CPack)
