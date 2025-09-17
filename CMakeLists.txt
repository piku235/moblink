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
  URL https://github.com/piku235/mobilus-gtw-client/archive/refs/tags/v0.3.0.zip
)
FetchContent_MakeAvailable(mobilus_gtw_client)

set(MOBILUS_CACERT_FILE "${mobilus_gtw_client_SOURCE_DIR}/certs/mobilelabs_mobilus_ca.crt")
file(READ "${MOBILUS_CACERT_FILE}" MOBILUS_CACERT)

configure_file(
  src/MobilusCacert.h.in
  "${CMAKE_BINARY_DIR}/src/MobilusCacert.h"
  @ONLY
)

add_executable(moblink
  src/filesystem/TempFile.cpp
  src/MqttMobilusGtwActor.cpp
  src/TargetMqttActor.cpp
  src/TargetMqttClient.cpp
  src/main.cpp
)

target_include_directories(moblink
  PRIVATE
    ${CMAKE_BINARY_DIR}/src
)

target_link_libraries(moblink
  PRIVATE
    mobilus_gtw_client
    OpenSSL::Crypto
    PkgConfig::MOSQUITTO
)
