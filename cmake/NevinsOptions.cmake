option(NEVINS_BUILD_GUI "Build the native dashboard binary" ON)
option(NEVINS_BUILD_TESTS "Build C++ tests" ON)
option(NEVINS_ENABLE_RTLSDR "Enable real RTL-SDR support through librtlsdr" OFF)
option(NEVINS_ENABLE_FSTAR_VERIFY "Run F*/KaRaMeL verification during the build" OFF)
option(NEVINS_ENABLE_SANITIZERS "Enable address and undefined behavior sanitizers" OFF)

function(nevins_apply_project_warnings target_name)
  if(MSVC)
    target_compile_options(${target_name} PRIVATE /W4 /WX)
  else()
    target_compile_options(${target_name} PRIVATE
      -Wall
      -Wextra
      -Wpedantic
      -Werror
    )
  endif()
endfunction()

function(nevins_apply_sanitizers target_name)
  if(NOT MSVC)
    target_compile_options(${target_name} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(${target_name} PRIVATE -fsanitize=address,undefined)
  endif()
endfunction()
