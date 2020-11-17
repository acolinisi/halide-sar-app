include(casper)

# TODO: this is also included from cac/CMakeLists.txt (revisit the design)
find_package(Halide REQUIRED)

find_library(CNPY cnpy REQUIRED)
find_path(CNPY_INCLUDE_DIR cnpy.h
          HINTS ${CMAKE_PREFIX_PATH}
          PATH_SUFFIXES include
          REQUIRED)

find_package(FFTW3 REQUIRED)

set(INPUT_DATA_DIR ../data/AFRL/pass1/HH_npy)

file(GLOB INPUT_DATA_FILES ${INPUT_DATA_DIR}/*)
set(INPUT_DATA_BLD_DIR input_data_dir)
file(MAKE_DIRECTORY ${INPUT_DATA_BLD_DIR})
foreach (f ${INPUT_DATA_FILES})
  get_filename_component(filename ${f} NAME)
  configure_file(${f} ${INPUT_DATA_BLD_DIR}/${filename} COPYONLY)
endforeach()

include_directories(..)

add_library(dft ../dft.cpp ../fftw.cpp)
#target_link_libraries(dft PUBLIC Halide::Halide Halide::ImageIO)

casper_add_exec(sarbp sarbp.meta
  SOURCES
    sarbp.meta.cpp
    ../backprojection.cpp
    ../img_output.cpp
    ../img_plane.cpp
  C_KERNEL_SOURCES kern.cpp ../PlatformData.cpp
  EXTRA_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/..
  EXTRA_PACKAGES FFTW3 Halide
  EXTRA_LIBRARIES
    ${CMAKE_CURRENT_BINARY_DIR}/libdft.a
    ${FFTW3_LIBRARIES}
    ${CNPY}
    Halide::Halide
    Halide::ImageIO
  PLATFORM platform.ini
  TUNED_PARAMS tuned-params.ini
)
target_link_libraries(sarbp.meta LINK_PUBLIC ${CNPY})

# TODO: try to expose ${target} as a target
add_dependencies(sarbp.link dft)

# Run the target program (test target)
add_custom_target(sarbp.run
  COMMAND ${sarbp_BUILD_DIR}/sarbp
  DEPENDS ${sarbp_BUILD_DIR}/sarbp)
