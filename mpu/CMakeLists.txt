cmake_minimum_required(VERSION 3.10)
project(audio_dsp_imx6ull VERSION 1.0.0 LANGUAGES C)

option(AUD_MPU_BUILD_RUNTIME "Build the ALSA runtime application" ON)
option(AUD_MPU_BUILD_TESTS "Build native unit tests" ON)
option(AUD_DSP_ENABLE_LGPL "Include the optional DaisySP LGPL modules" OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

get_filename_component(AUDIO_DSP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/portable_sources.cmake")

if(AUD_DSP_ENABLE_LGPL)
  list(APPEND AUDIO_DSP_SOURCES ${AUDIO_DSP_LGPL_SOURCES})
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
    src/aud_mpu_config.c
    src/aud_mpu_io_state.c
    src/aud_mpu_pcm.c
    src/aud_mpu_fx_chain.c)
target_include_directories(aud_mpu_core PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${AUDIO_DSP_ROOT}")
target_compile_options(aud_mpu_core PRIVATE -Wall -Wextra -Werror)
target_link_libraries(aud_mpu_core PUBLIC audio_dsp_portable m)

if(AUD_MPU_BUILD_RUNTIME)
  find_path(ALSA_INCLUDE_DIR NAMES alsa/asoundlib.h)
  find_library(ALSA_LIBRARY NAMES asound)
  if(NOT ALSA_INCLUDE_DIR OR NOT ALSA_LIBRARY)
    message(FATAL_ERROR "ALSA development headers/library were not found")
  endif()
  find_package(Threads REQUIRED)

  add_library(aud_mpu_alsa STATIC src/aud_mpu_alsa.c)
  target_include_directories(aud_mpu_alsa PUBLIC
      "${CMAKE_CURRENT_SOURCE_DIR}/include"
      "${ALSA_INCLUDE_DIR}")
  target_compile_options(aud_mpu_alsa PRIVATE -Wall -Wextra -Werror)
  target_compile_definitions(aud_mpu_alsa PUBLIC _POSIX_C_SOURCE=200809L)
  target_link_libraries(aud_mpu_alsa PUBLIC aud_mpu_core "${ALSA_LIBRARY}")

  add_executable(audio_dsp_imx6ull app/audio_dsp_imx6ull.c)
  target_compile_options(audio_dsp_imx6ull PRIVATE -Wall -Wextra -Werror)
  target_compile_definitions(audio_dsp_imx6ull PRIVATE
      AUD_MPU_VERSION="${PROJECT_VERSION}")
  target_link_libraries(audio_dsp_imx6ull PRIVATE
      aud_mpu_alsa Threads::Threads)

  install(TARGETS audio_dsp_imx6ull
      RUNTIME DESTINATION /usr/bin COMPONENT Runtime)
  install(FILES config/audio-dsp.conf
      DESTINATION /usr/share/audio-dsp-imx6ull
      RENAME audio-dsp.conf.default COMPONENT Runtime)
  install(PROGRAMS scripts/board-check.sh
      DESTINATION /usr/libexec/audio-dsp-imx6ull
      RENAME board-check COMPONENT Runtime)
  install(PROGRAMS scripts/uninstall-target.sh
      DESTINATION /usr/libexec/audio-dsp-imx6ull
      RENAME uninstall COMPONENT Runtime)
  install(FILES packaging/audio-dsp-imx6ull.service
      DESTINATION /lib/systemd/system COMPONENT Runtime)
  install(FILES board/100ask-imx6ull/profile.env
      DESTINATION /usr/share/audio-dsp-imx6ull COMPONENT Runtime)
  install(PROGRAMS packaging/audio-dsp-imx6ull.init
      DESTINATION /etc/init.d RENAME audio-dsp-imx6ull COMPONENT Runtime)
  install(FILES "${AUDIO_DSP_ROOT}/LICENSE" "${AUDIO_DSP_ROOT}/THIRD_PARTY_NOTICES.md"
      DESTINATION /usr/share/doc/audio-dsp-imx6ull COMPONENT Runtime)
  install(DIRECTORY "${AUDIO_DSP_ROOT}/LICENSES/"
      DESTINATION /usr/share/doc/audio-dsp-imx6ull/LICENSES COMPONENT Runtime)
  if(EXISTS "${CMAKE_BINARY_DIR}/ELF_ABI.txt")
    install(FILES "${CMAKE_BINARY_DIR}/ELF_ABI.txt"
        DESTINATION /usr/share/doc/audio-dsp-imx6ull COMPONENT Runtime)
  endif()
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

  add_executable(test_mpu_config tests/test_mpu_config.c)
  target_compile_options(test_mpu_config PRIVATE -Wall -Wextra -Werror)
  target_link_libraries(test_mpu_config PRIVATE aud_mpu_core)
  add_test(NAME mpu_config COMMAND test_mpu_config)

  add_executable(test_mpu_io_state tests/test_mpu_io_state.c)
  target_compile_options(test_mpu_io_state PRIVATE -Wall -Wextra -Werror)
  target_link_libraries(test_mpu_io_state PRIVATE aud_mpu_core)
  add_test(NAME mpu_io_state COMMAND test_mpu_io_state)

  add_test(NAME mpu_package_validation
      COMMAND sh "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_package_validation.sh")
endif()
