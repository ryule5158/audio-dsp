set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)

# i.MX6ULL contains a Cortex-A7 with NEON/VFPv4 and uses the hard-float ABI.
set(CMAKE_C_FLAGS_INIT
    "-mcpu=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard")

# AUD_MPU_SYSROOT may be a complete target sysroot or Ubuntu's multiarch root.
# It is a CMake search root only: the distro cross compiler keeps using its own
# tested libc sysroot unless the caller explicitly adds --sysroot in CFLAGS.
if(DEFINED ENV{AUD_MPU_SYSROOT} AND NOT "$ENV{AUD_MPU_SYSROOT}" STREQUAL "")
  set(CMAKE_FIND_ROOT_PATH "$ENV{AUD_MPU_SYSROOT}")
endif()
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
