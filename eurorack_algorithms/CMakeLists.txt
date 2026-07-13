cmake_minimum_required(VERSION 3.20)
cmake_policy(SET CMP0123 NEW)
project(aud_eurorack_h7 LANGUAGES C CXX)

option(AUD_EURO_BUILD_EXAMPLES "Compile portable C usage examples" ON)
option(AUD_EURO_BUILD_LINK_SMOKE "Link all public modules into one H7 image" ON)

set(VENDOR_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor")
set(PORT_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/port/include")
set(PUBLIC_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

set(AUD_EURO_COMPILE_OPTIONS
    -mcpu=cortex-m7
    -mthumb
    -mfpu=fpv5-d16
    -mfloat-abi=hard
    -O2
    -ffast-math
    -ffunction-sections
    -fdata-sections
    -Wall
    -Wextra
    -Wno-unused-parameter
    -Wno-unused-variable
    -Wno-sign-compare
    -Wno-unused-local-typedefs)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  list(APPEND AUD_EURO_COMPILE_OPTIONS -Wno-maybe-uninitialized)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  list(APPEND AUD_EURO_COMPILE_OPTIONS
      -Wno-unknown-warning-option
      -Wno-unused-private-field
      -Wno-unused-but-set-variable
      -Wno-vla-cxx-extension)
endif()

function(add_aud_euro_module target)
  add_library(${target} OBJECT ${ARGN})
  target_include_directories(${target} PRIVATE
      "${PUBLIC_INCLUDE_DIR}" "${PORT_INCLUDE_DIR}" "${VENDOR_DIR}")
  target_compile_definitions(${target} PRIVATE
      GCC_ARMCM7
      ARM_MATH_CM7
      __FPU_PRESENT=1
      AUD_EURO_DISABLE_PERSISTENCE=1)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_definitions(${target} PRIVATE
        M_PI=3.14159265358979323846)
  endif()
  target_compile_options(${target} PRIVATE ${AUD_EURO_COMPILE_OPTIONS})
  target_compile_options(${target} PRIVATE
      $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
      $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
      $<$<COMPILE_LANGUAGE:CXX>:-fno-threadsafe-statics>
      $<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit>)
endfunction()

file(GLOB BRAIDS_SOURCES CONFIGURE_DEPENDS "${VENDOR_DIR}/braids/*.cc")
file(GLOB_RECURSE CLOUDS_SOURCES CONFIGURE_DEPENDS "${VENDOR_DIR}/clouds/dsp/*.cc")
file(GLOB_RECURSE ELEMENTS_SOURCES CONFIGURE_DEPENDS "${VENDOR_DIR}/elements/dsp/*.cc")
file(GLOB_RECURSE MARBLES_SOURCES CONFIGURE_DEPENDS
    "${VENDOR_DIR}/marbles/random/*.cc"
    "${VENDOR_DIR}/marbles/ramp/*.cc")
file(GLOB_RECURSE PEAKS_SOURCES CONFIGURE_DEPENDS
    "${VENDOR_DIR}/peaks/drums/*.cc"
    "${VENDOR_DIR}/peaks/modulations/*.cc"
    "${VENDOR_DIR}/peaks/number_station/*.cc"
    "${VENDOR_DIR}/peaks/pulse_processor/*.cc")
file(GLOB_RECURSE PLAITS_SOURCES CONFIGURE_DEPENDS "${VENDOR_DIR}/plaits/dsp/*.cc")
file(GLOB_RECURSE RINGS_SOURCES CONFIGURE_DEPENDS "${VENDOR_DIR}/rings/dsp/*.cc")
file(GLOB_RECURSE WARPS_SOURCES CONFIGURE_DEPENDS "${VENDOR_DIR}/warps/dsp/*.cc")
set(TIDES2_SOURCES
    "${VENDOR_DIR}/tides2/poly_slope_generator.cc"
    "${VENDOR_DIR}/tides2/ramp/ramp_extractor.cc"
    "${VENDOR_DIR}/tides2/resources.cc")

# Both Tides generations export `tides::lookup_table_table`. Rename only the
# Tides 2 table, not its namespace: Stages intentionally links against the
# Tides 2 `tides::RampExtractor` implementation.
set_source_files_properties(
    ${TIDES2_SOURCES}
    "${CMAKE_CURRENT_SOURCE_DIR}/port/src/aud_euro_tides2.cc"
    PROPERTIES COMPILE_DEFINITIONS
        "lookup_table_table=tides2_lookup_table_table")

add_aud_euro_module(aud_euro_common
    "${VENDOR_DIR}/stmlib/dsp/atan.cc"
    "${VENDOR_DIR}/stmlib/dsp/units.cc"
    "${VENDOR_DIR}/stmlib/utils/random.cc")
add_aud_euro_module(aud_euro_braids ${BRAIDS_SOURCES})
add_aud_euro_module(aud_euro_clouds ${CLOUDS_SOURCES} "${VENDOR_DIR}/clouds/resources.cc")
add_aud_euro_module(aud_euro_elements ${ELEMENTS_SOURCES} "${VENDOR_DIR}/elements/resources.cc")
add_aud_euro_module(aud_euro_frames
    "${VENDOR_DIR}/frames/keyframer.cc"
    "${VENDOR_DIR}/frames/poly_lfo.cc"
    "${VENDOR_DIR}/frames/resources.cc")
add_aud_euro_module(aud_euro_fsk
    "${VENDOR_DIR}/stm_audio_bootloader/fsk/packet_decoder.cc")
add_aud_euro_module(aud_euro_marbles ${MARBLES_SOURCES} "${VENDOR_DIR}/marbles/resources.cc")
add_aud_euro_module(aud_euro_peaks ${PEAKS_SOURCES}
    "${VENDOR_DIR}/peaks/processors.cc"
    "${VENDOR_DIR}/peaks/resources.cc")
add_aud_euro_module(aud_euro_plaits ${PLAITS_SOURCES} "${VENDOR_DIR}/plaits/resources.cc")
add_aud_euro_module(aud_euro_rings ${RINGS_SOURCES} "${VENDOR_DIR}/rings/resources.cc")
add_aud_euro_module(aud_euro_stages
    "${VENDOR_DIR}/stages/segment_generator.cc"
    "${VENDOR_DIR}/stages/resources.cc")
add_aud_euro_module(aud_euro_streams
    "${VENDOR_DIR}/streams/compressor.cc"
    "${VENDOR_DIR}/streams/envelope.cc"
    "${VENDOR_DIR}/streams/follower.cc"
    "${VENDOR_DIR}/streams/lorenz_generator.cc"
    "${VENDOR_DIR}/streams/processor.cc"
    "${VENDOR_DIR}/streams/svf.cc"
    "${VENDOR_DIR}/streams/vactrol.cc"
    "${VENDOR_DIR}/streams/resources.cc")
add_aud_euro_module(aud_euro_tides
    "${VENDOR_DIR}/tides/generator.cc"
    "${VENDOR_DIR}/tides/plotter.cc"
    "${VENDOR_DIR}/tides/resources.cc")
add_aud_euro_module(aud_euro_tides2 ${TIDES2_SOURCES})
add_aud_euro_module(aud_euro_warps ${WARPS_SOURCES} "${VENDOR_DIR}/warps/resources.cc")
add_aud_euro_module(aud_euro_grids
    "${VENDOR_DIR}/grids/clock.cc"
    "${VENDOR_DIR}/grids/pattern_generator.cc"
    "${VENDOR_DIR}/grids/resources.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/port/src/avrlib_random.cc")
add_aud_euro_module(aud_euro_yarns
    "${VENDOR_DIR}/yarns/just_intonation_processor.cc"
    "${VENDOR_DIR}/yarns/resources.cc")
add_aud_euro_module(aud_euro_c_api
    "${CMAKE_CURRENT_SOURCE_DIR}/port/src/aud_euro_branches.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/port/src/aud_euro_braids.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/port/src/aud_euro_clouds.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/port/src/aud_euro_elements.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/port/src/aud_euro_edges.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/port/src/aud_euro_fsk.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/port/src/aud_euro_frames.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/port/src/aud_euro_grids.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/port/src/aud_euro_marbles.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/port/src/aud_euro_peaks.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/port/src/aud_euro_plaits.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/port/src/aud_euro_rings.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/port/src/aud_euro_stages.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/port/src/aud_euro_streams.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/port/src/aud_euro_tides.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/port/src/aud_euro_tides2.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/port/src/aud_euro_warps.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/port/src/aud_euro_yarns.cc")

set(AUD_EURO_OBJECT_TARGETS
    aud_euro_common aud_euro_braids aud_euro_clouds aud_euro_elements
    aud_euro_frames aud_euro_fsk aud_euro_marbles aud_euro_peaks aud_euro_plaits
    aud_euro_rings aud_euro_stages aud_euro_streams aud_euro_tides
    aud_euro_tides2 aud_euro_warps aud_euro_grids aud_euro_yarns aud_euro_c_api)

set(AUD_EURO_OBJECTS)
foreach(target IN LISTS AUD_EURO_OBJECT_TARGETS)
  list(APPEND AUD_EURO_OBJECTS $<TARGET_OBJECTS:${target}>)
endforeach()

add_library(aud_eurorack_h7 STATIC ${AUD_EURO_OBJECTS})
set_target_properties(aud_eurorack_h7 PROPERTIES OUTPUT_NAME aud_eurorack_h7)
target_include_directories(aud_eurorack_h7 PUBLIC "${PUBLIC_INCLUDE_DIR}")

if(AUD_EURO_BUILD_EXAMPLES)
  file(GLOB_RECURSE AUD_EURO_EXAMPLE_SOURCES CONFIGURE_DEPENDS
      "${CMAKE_CURRENT_SOURCE_DIR}/examples/*.c")
  add_library(aud_euro_examples OBJECT ${AUD_EURO_EXAMPLE_SOURCES})
  target_include_directories(aud_euro_examples PRIVATE "${PUBLIC_INCLUDE_DIR}")
  target_compile_options(aud_euro_examples PRIVATE ${AUD_EURO_COMPILE_OPTIONS})
endif()

if(AUD_EURO_BUILD_LINK_SMOKE AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  add_executable(aud_euro_h7_link_smoke
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/h7_api_link_smoke.c")
  set_target_properties(aud_euro_h7_link_smoke PROPERTIES LINKER_LANGUAGE CXX)
  target_compile_options(aud_euro_h7_link_smoke PRIVATE ${AUD_EURO_COMPILE_OPTIONS})
  target_link_options(aud_euro_h7_link_smoke PRIVATE
      -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard
      -nostartfiles
      "-T${CMAKE_CURRENT_SOURCE_DIR}/tests/h7_smoke.ld"
      -Wl,--gc-sections
      -Wl,-Map=aud_euro_h7_link_smoke.map)
  target_link_libraries(aud_euro_h7_link_smoke PRIVATE
      aud_eurorack_h7 m c gcc nosys)
endif()
