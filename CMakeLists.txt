cmake_minimum_required(VERSION 3.22)
project(btor_analyzer)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR )
    message (FATAL_ERROR
            "In-source builds are not allowed. Please clean your source tree and try again.")
endif()

# Add path for custom modules
set(CMAKE_MODULE_PATH
        ${CMAKE_MODULE_PATH}
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
        )

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#add_definitions(-D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS)

option(BTOR_ANALYZER_STATIC_EXE "Static executables." OFF)

#set (CUSTOM_BOOST_ROOT "" CACHE PATH "Path to custom boost installation.")
#if (CUSTOM_BOOST_ROOT)
#    set (BOOST_ROOT ${CUSTOM_BOOST_ROOT})
#    set (Boost_NO_SYSTEM_PATHS "ON")
#endif()

set (Boost_USE_STATIC_LIBS ON)
find_package (Boost 1.61 REQUIRED program_options)
IF (Boost_FOUND)
    include_directories (${Boost_INCLUDE_DIRS})
endif ()

# prefer linking with static libraries
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a" ${CMAKE_FIND_LIBRARY_SUFFIXES})

# check for rt lib. Not needed on OSX.
find_library(RT_LIB NAMES rt)
if (NOT RT_LIB)
    set(RT_LIB "")
endif()
mark_as_advanced(RT_LIB)

find_package(Abc REQUIRED)
if (ABC_FOUND)
    include_directories(${ABC_INCLUDE_DIR})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ABC_CXXFLAGS}")
    add_definitions (-DABC_NAMESPACE=${ABC_NAMESPACE})
else()
    set(ABC_LIBRARY "")
endif ()

#set(SMTPARSER_LIBRARY /home/yakir/workspace/bvmc/smtlib2parser-1.4/libsmtlib2parser.a)
set(BTOR_LIBRARY ${BTOR_ROOT}/build/lib/libbtor2parser.a)

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}/include
        ${BTOR_ROOT}/src/btor2parser
)

add_subdirectory(src)
