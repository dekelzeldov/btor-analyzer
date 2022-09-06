add_library (BtorAnalyzer STATIC metadata.cpp)

set (EXT_LIBRARIES ${ABC_LIBRARY})

add_executable (btor-analyzer main.cpp)
target_link_libraries (btor-analyzer
#  AbcCpp
  BtorAnalyzer
  ${AVY_DEBUG_LIBRARY}
  ${EXT_LIBRARIES}
  ${BTOR_PARSER_LIBRARY}
  ${BTOR_AIGER_LIBRARY}
  Boolector::boolector
  ${GLUCOSE_LIBRARY}
 # ${Boost_PROGRAM_OPTIONS_LIBRARY}
  m dl pthread z gmp gmpxx stdc++
  ${RT_LIB})


if (ANALYZER_STATIC_EXE)
    set (CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")
    set_target_properties (btor-analyzer PROPERTIES LINK_SEARCH_START_STATIC ON)
    set_target_properties (btor-analyzer PROPERTIES LINK_SEARCH_END_STATIC ON)
endif()
