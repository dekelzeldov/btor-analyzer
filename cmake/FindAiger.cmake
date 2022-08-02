# Find Aiger

set(AIGER_ROOT "" CACHE PATH "Root of AIGER compiled source tree.")

set(AIGER_NAMESPACE "aiger" CACHE STRING "Aiger namespace to use.")

if (DEFINED AIGER_SOURCE_DIR)
  include ("${AIGER_SOURCE_DIR}/cmake/PackageOptions.cmake")
  set (AIGER_LIBRARY aiger-static-lib)
else()
  find_program(AIGER_ARCH_FLAGS NAMES arch_flags PATHS ${AIGER_ROOT})
  if (AIGER_ARCH_FLAGS)
    execute_process (COMMAND ${AIGER_ARCH_FLAGS}
      OUTPUT_VARIABLE AIGER_CXXFLAGS
      OUTPUT_STRIP_TRAILING_WHITESPACE )

    set(AIGER_CXXFLAGS "${AIGER_CXXFLAGS} -DAIGER_NAMESPACE=aiger")

    message (STATUS "AIGER arch flags are: ${AIGER_CXXFLAGS}")
  endif()

  find_path(AIGER_INCLUDE_DIR NAMES base/aiger/aiger.h PATHS ${AIGER_ROOT}/src)
  find_library(AIGER_LIBRARY NAMES aiger PATHS ${AIGER_ROOT})
endif()


include (FindPackageHandleStandardArgs)
find_package_handle_standard_args(Aiger
  REQUIRED_VARS AIGER_LIBRARY AIGER_INCLUDE_DIR)

mark_as_advanced(AIGER_LIBRARY AIGER_INCLUDE_DIR AIGER_CXXFLAGS AIGER_ARCH_FLAGS)