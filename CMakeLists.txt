cmake_minimum_required(VERSION 3.25)

project(
  NevinsPort
  VERSION 0.1.0
  DESCRIPTION "Lawful receive-only radio operations dashboard for RTL-SDR"
  LANGUAGES CXX
)

include(cmake/NevinsOptions.cmake)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(nevins_core STATIC
  src/capture.cpp
  src/core_bridge.cpp
  src/device_mock.cpp
  src/device_rtlsdr.cpp
  src/signal_card.cpp
  src/spectrum.cpp
)

target_include_directories(nevins_core
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(nevins_core PUBLIC cxx_std_23)
nevins_apply_project_warnings(nevins_core)

target_compile_definitions(nevins_core
  PUBLIC
    NEVINS_VERSION="${PROJECT_VERSION}"
)

if(NEVINS_ENABLE_RTLSDR)
  find_package(PkgConfig QUIET)
  target_compile_definitions(nevins_core PUBLIC NEVINS_ENABLE_RTLSDR=1)
  if(PkgConfig_FOUND)
    pkg_check_modules(RTLSDR QUIET IMPORTED_TARGET rtlsdr)
  endif()
  if(TARGET PkgConfig::RTLSDR)
    target_link_libraries(nevins_core PUBLIC PkgConfig::RTLSDR)
  else()
    find_path(RTLSDR_INCLUDE_DIR NAMES rtl-sdr.h)
    find_library(RTLSDR_LIBRARY NAMES rtlsdr)
    if(NOT RTLSDR_INCLUDE_DIR OR NOT RTLSDR_LIBRARY)
      message(FATAL_ERROR "NEVINS_ENABLE_RTLSDR=ON requires rtl-sdr.h and librtlsdr")
    endif()
    target_include_directories(nevins_core PUBLIC ${RTLSDR_INCLUDE_DIR})
    target_link_libraries(nevins_core PUBLIC ${RTLSDR_LIBRARY})
  endif()
else()
  target_compile_definitions(nevins_core PUBLIC NEVINS_ENABLE_RTLSDR=0)
endif()

if(NEVINS_ENABLE_SANITIZERS)
  nevins_apply_sanitizers(nevins_core)
endif()

add_executable(nevins apps/nevins/main.cpp)
target_link_libraries(nevins PRIVATE nevins_core)
nevins_apply_project_warnings(nevins)
set_target_properties(nevins PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/apps/nevins
)

if(NEVINS_BUILD_GUI)
  find_package(OpenGL REQUIRED)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(GLFW3 REQUIRED IMPORTED_TARGET glfw3)

  add_executable(nevins-console apps/nevins-console/main.cpp)
  target_link_libraries(nevins-console PRIVATE nevins_core OpenGL::GL PkgConfig::GLFW3)
  nevins_apply_project_warnings(nevins-console)
  set_target_properties(nevins-console PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/apps/nevins-console
  )
endif()

if(NEVINS_ENABLE_FSTAR_VERIFY)
  add_custom_target(verify_fstar ALL
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/verify-fstar.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Verifying F* core specifications"
    VERBATIM
  )
endif()

if(NEVINS_BUILD_TESTS)
  enable_testing()

  file(GLOB NEVINS_TEST_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_*.cpp
  )

  foreach(test_source IN LISTS NEVINS_TEST_SOURCES)
    get_filename_component(test_name ${test_source} NAME_WE)
    add_executable(${test_name} ${test_source})
    target_link_libraries(${test_name} PRIVATE nevins_core)
    nevins_apply_project_warnings(${test_name})
    set_target_properties(${test_name} PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
    )
    add_test(NAME ${test_name} COMMAND ${CMAKE_BINARY_DIR}/tests/${test_name})
    set_tests_properties(${test_name} PROPERTIES
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
  endforeach()
endif()
