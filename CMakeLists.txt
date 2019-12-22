cmake_policy(SET CMP0048 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0048 NEW)
cmake_minimum_required(VERSION 3.12)

# Shared library versioning is done in the same way as SDL:
#
# If the version changes, then:
#
#   NCPP_VERSION_MICRO += 1;
#   NCPP_INTERFACE_AGE += 1;
#   NCPP_BINARY_AGE += 1;
# if any functions have been added, set NCPP_INTERFACE_AGE to 0.
# if backwards compatibility has been broken,
# set NCPP_BINARY_AGE and NCPP_INTERFACE_AGE to 0.

set(NCPP_VERSION_MAJOR 0)
set(NCPP_VERSION_MINOR 0)
set(NCPP_VERSION_MICRO 1)

set(NCPP_INTERFACE_AGE 0)
set(NCPP_BINARY_AGE 1)

set(NCPP_VERSION "${NCPP_VERSION_MAJOR}.${NCPP_VERSION_MINOR}.${NCPP_VERSION_MICRO}")

# Calculate a libtool-like version number (taken from SDL2 CMakeLists.txt)
math(EXPR LT_CURRENT "${NCPP_VERSION_MICRO} - ${NCPP_INTERFACE_AGE}")
math(EXPR LT_AGE "${NCPP_BINARY_AGE} - ${NCPP_INTERFACE_AGE}")
math(EXPR LT_MAJOR "${LT_CURRENT}- ${LT_AGE}")
set(LT_REVISION "${NCPP_INTERFACE_AGE}")
set(LT_RELEASE "${NCPP_VERSION_MAJOR}.${NCPP_VERSION_MINOR}")
set(LT_VERSION "${LT_MAJOR}.${LT_AGE}.${LT_REVISION}")

project(
  notcurses++
  VERSION ${NCPP_VERSION_MAJOR}.${NCPP_VERSION_MINOR}.${NCPP_VERSION_MICRO}
  DESCRIPTION "notcurses C++ wrapper"
  LANGUAGES CXX
  )

set(PACKAGE_NAME ${PROJECT_NAME})
set(PACKAGE_VERSION ${PROJECT_VERSION})
set(PACKAGE_STRING "${PROJECT_NAME} v${PROJECT_VERSION}")
set(PACKAGE_VERSION_MAJOR "${NCPP_VERSION_MAJOR}")
set(PACKAGE_VERSION_MINOR "${NCPP_VERSION_MINOR}")
set(PACKAGE_VERSION_MICRO "${NCPP_VERSION_MICRO}")

option(OPTIMIZE_NATIVE "Optimize for the native (host) compiler" ON)
option(USE_LTO "Use Link-Time Optimizations if the compiler supports them" ON)
option(USE_LLD "Use the LLVM lld linker, if found" OFF)
option(ENABLE_PROFILING "Support profiling of the generated code" OFF)
option(COMPILER_DIAG_COLOR "Show compiler diagnostics/errors in color" ON)

include(CheckIncludeFiles)
include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
include(FindPkgConfig)

if (!PKG_CONFIG_FOUND)
  message(FATAL_ERROR "pkg-config is required")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_BUILD_RPATH "${CMAKE_BINARY_DIR}/lib")

set(CMAKE_SKIP_INSTALL_RPATH TRUE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

macro(cxx_compiler_has_flag _flag)
  string(REGEX REPLACE "-|,|=" "_" _flag_name ${_flag})
  check_cxx_compiler_flag(-${_flag} HAS_${_flag_name}_CXX)
  if (HAS_${_flag_name}_CXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -${_flag}")
  endif()
endmacro(cxx_compiler_has_flag)

set(TEST_COMPILER_ARGS
  fno-strict-aliasing
  ffunction-sections
  funswitch-loops
  finline-limit=300
  fstack-protector
  Wformat
  Werror=format-security
  Wall
  Wextra
  Wnull-dereference
  Wmisleading-indentation
  Wunused
  Wpedantic
#  Wsuggest-final-types
#  Wsuggest-final-methods
  Wsuggest-override
  g
  fvisibility=hidden
  fno-rtti
  mpopcnt
  fpic
  )

if (NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(TEST_COMPILER_ARGS
	${TEST_COMPILER_ARGS}
	rdynamic
	Wa,--noexecstack
	)
endif()

if(COMPILER_DIAG_COLOR)
  set(TEST_COMPILER_ARGS ${TEST_COMPILER_ARGS} fdiagnostics-color=always)
else()
  set(TEST_COMPILER_ARGS ${TEST_COMPILER_ARGS} fdiagnostics-color=auto)
endif()

if(OPTIMIZE_NATIVE)
  set(TEST_COMPILER_ARGS ${TEST_COMPILER_ARGS}
    march=native
    mtune=native
    )
endif()

if(ENABLE_PROFILING)
  set(TEST_COMPILER_ARGS ${TEST_COMPILER_ARGS}
    pg
    )
endif()

if(USE_LTO)
  set(TEST_COMPILER_ARGS ${TEST_COMPILER_ARGS} flto)
endif()

if(USE_LLD)
  set(TEST_COMPILER_ARGS ${TEST_COMPILER_ARGS} fuse-ld=lld)
else()
  if (NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	set(TEST_COMPILER_ARGS ${TEST_COMPILER_ARGS} fuse-ld=gold)
  endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL Debug)
  add_definitions("-DDEBUG=1")
  set(TEST_COMPILER_ARGS ${TEST_COMPILER_ARGS} ggdb3 fno-omit-frame-pointer)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
else()
  add_definitions("-DNDEBUG")
  set(TEST_COMPILER_ARGS ${TEST_COMPILER_ARGS} omit-frame-pointer flto)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif()

foreach(arg ${TEST_COMPILER_ARGS})
  cxx_compiler_has_flag(${arg})
endforeach(arg)

# These should be tested for, but currently it's not that easy with cmake so let's hope we'll get lucky :P
if (NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(TEST_LINKER_ARGS "-Wl,-z,now -Wl,-z,relro -Wl,-z,noexecstack -Wl,--no-undefined")
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TEST_LINKER_ARGS}")

include_directories(include ${CMAKE_CURRENT_BINARY_DIR}/include/)

if(CMAKE_HOST_SYSTEM_NAME STREQUAL Linux)
  add_definitions(-D_GNU_SOURCE)
endif()

pkg_check_modules(AVUTIL REQUIRED libavutil>=56.0)
pkg_check_modules(AVFORMAT REQUIRED libavformat>=57.0)
pkg_check_modules(NOTCURSES REQUIRED notcurses>=0.9.2)

set(NOTCURSES_DATA_DIR "${NOTCURSES_PREFIX}/share/notcurses")
include_directories(${NOTCURSES_INCLUDE_DIRS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${NOTCURSES_CFLAGS} ${NOTCURSES_CFLAGS_OTHER}")

add_subdirectory(src/lib)
add_subdirectory(src/poc)
add_subdirectory(src/view)
add_subdirectory(src/planereel)

message(STATUS "")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Compiler flags: ${CMAKE_CXX_FLAGS}")
message(STATUS "")

configure_file(src/config.hh.in ${CMAKE_CURRENT_BINARY_DIR}/include/config.hh)
