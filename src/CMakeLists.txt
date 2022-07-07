#add_library (BvMc STATIC BvMc.cpp)

set (EXT_LIBRARIES ${ABC_LIBRARY})

add_executable (btor-analyzer main.cpp metadata.h)
#add_executable (lia2bv smtlib2bv.cc)
target_link_libraries (btor-analyzer
#  AbcCpp
#  AvyDebug
  ${EXT_LIBRARIES}
  ${BTOR_LIBRARY}
 # ${Boost_PROGRAM_OPTIONS_LIBRARY}
  m dl pthread z gmp gmpxx stdc++
  ${RT_LIB})


if (BVMC_STATIC_EXE)
    set (CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")
    set_target_properties (btor-analyzer PROPERTIES LINK_SEARCH_START_STATIC ON)
    set_target_properties (btor-analyzer PROPERTIES LINK_SEARCH_END_STATIC ON)
endif()
