set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR mipsel)

set(CMAKE_C_COMPILER mipsel-openwrt-linux-gcc)
set(CMAKE_CXX_COMPILER mipsel-openwrt-linux-g++)
set(CMAKE_EXE_LINKER_FLAGS "-Wl,-rpath=/opt/jungi/lib -Wl,--dynamic-linker=/opt/jungi/lib/ld-musl-mipsel-sf.so.1")

set(CROSS_C_FLAGS "-march=24kec -mxgot")
set(CROSS_CXX_FLAGS "-march=24kec -mxgot")
set(CMAKE_C_FLAGS_INIT "${CROSS_C_FLAGS}")
set(CMAKE_CXX_FLAGS_INIT "${CROSS_CXX_FLAGS}")

if(NOT DEFINED CMAKE_SYSROOT)
  message(FATAL_ERROR "CMAKE_SYSROOT must be provided")
endif()

set(ENV{PKG_CONFIG_SYSROOT_DIR} ${CMAKE_SYSROOT})
set(ENV{PKG_CONFIG_PATH} "${CMAKE_SYSROOT}/usr/lib/pkgconfig")

set(CMAKE_FIND_ROOT_PATH ${CMAKE_SYSROOT})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
