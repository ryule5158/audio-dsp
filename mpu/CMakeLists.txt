cmake_minimum_required(VERSION 3.10)
project(audio_dsp_imx6ull C)

option(AUD_MPU_BUILD_ALSA "Build ALSA runtime and target application" ON)
option(AUD_MPU_BUILD_TESTS "Build host unit tests" ON)
option(AUD_DSP_ENABLE_LGPL "Include DaisySP-LGPL-derived root modules" OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

get_filename_component(AUDIO_DSP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
file(GLOB AUDIO_DSP_SOURCES "${AUDIO_DSP_ROOT}/aud_*.c")

set(AUDIO_DSP_LGPL_BASENAMES
    aud_allpass.c aud_balance.c aud_bitcrush.c aud_blosc.c aud_comb.c
    aud_compressor.c aud_fold.c aud_jitter.c aud_line.c aud_mode.c
    aud_moogladder.c aud_nlfilt.c aud_pluck.c aud_polypluck.c aud_port.c
    aud_reverbsc.c aud_tone.c)

if(NOT AUD_DSP_ENABLE_LGPL)
  foreach(name IN LISTS AUDIO_DSP_LGPL_BASENAMES)
    list(REMOVE_ITEM AUDIO_DSP_SOURCES "${AUDIO_DSP_ROOT}/${name}")
  endforeach()
endif()

add_library(audio_dsp_portable STATIC ${AUDIO_DSP_SOURCES})
if(AUD_DSP_ENABLE_LGPL)
  set_target_properties(audio_dsp_portable PROPERTIES
      OUTPUT_NAME audio_dsp_portable_lgpl)
endif()
target_include_directories(audio_dsp_portable PUBLIC "${AUDIO_DSP_ROOT}")
target_compile_options(audio_dsp_portable PRIVATE
    -Wall -Wextra -Wno-misleading-indentation)

add_library(aud_mpu_core STATIC
    src/aud_mpu_pcm.c
    src/aud_mpu_fx_chain.c)
target_include_directories(aud_mpu_core PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${AUDIO_DSP_ROOT}")
target_compile_options(aud_mpu_core PRIVATE -Wall -Wextra -Werror)
target_link_libraries(aud_mpu_core PUBLIC audio_dsp_portable m)

if(AUD_MPU_BUILD_ALSA)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(ALSA REQUIRED alsa)
  find_package(Threads REQUIRED)

  add_library(aud_mpu_alsa STATIC src/aud_mpu_alsa.c)
  target_include_directories(aud_mpu_alsa PUBLIC
      "${CMAKE_CURRENT_SOURCE_DIR}/include"
      ${ALSA_INCLUDE_DIRS})
  target_compile_options(aud_mpu_alsa PRIVATE -Wall -Wextra -Werror)
  # ALSA 1.1.x on Ubuntu 18.04 conditionally supplies struct timespec unless
  # a POSIX feature level is visible.  Export the definition to consumers of
  # the public header as well as to the runtime itself.
  target_compile_definitions(aud_mpu_alsa PUBLIC _POSIX_C_SOURCE=200809L)
  target_link_libraries(aud_mpu_alsa PUBLIC aud_mpu_core ${ALSA_LIBRARIES})

  add_executable(audio_dsp_imx6ull app/audio_dsp_imx6ull.c)
  target_compile_options(audio_dsp_imx6ull PRIVATE -Wall -Wextra -Werror)
  target_link_libraries(audio_dsp_imx6ull PRIVATE
      aud_mpu_alsa Threads::Threads)
endif()

if(AUD_MPU_BUILD_TESTS AND NOT CMAKE_CROSSCOMPILING)
  enable_testing()
  add_executable(test_mpu_pcm tests/test_mpu_pcm.c)
  target_compile_options(test_mpu_pcm PRIVATE -Wall -Wextra -Werror)
  target_link_libraries(test_mpu_pcm PRIVATE aud_mpu_core)
  add_test(NAME mpu_pcm COMMAND test_mpu_pcm)

  add_executable(test_mpu_fx tests/test_mpu_fx.c)
  target_compile_options(test_mpu_fx PRIVATE -Wall -Wextra -Werror)
  target_link_libraries(test_mpu_fx PRIVATE aud_mpu_core)
  add_test(NAME mpu_fx COMMAND test_mpu_fx)
endif()
