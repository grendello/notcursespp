set(NCPP_SOURCES
  NotCurses.cc
  Plane.cc
  Root.cc
  )

add_library(
  notcurses++
  SHARED
  ${NCPP_SOURCES}
  )

if(APPLE)
  set_target_properties(notcurses++ PROPERTIES
    MACOSX_RPATH 1
    OUTPUT_NAME "notcurses++-${LT_RELEASE}")
elseif(UNIX)
  set_target_properties(notcurses++ PROPERTIES
    VERSION ${LT_VERSION}
    SOVERSION ${LT_REVISION}
    OUTPUT_NAME "notcurses++-${LT_RELEASE}")
else()                                                                                                                                                                                                                                                                                                   
  message(FATAL_ERROR "Only Unix supported, sorry")
endif()

target_link_libraries(notcurses++ ${NOTCURSES_LDFLAGS} ${NOTCURSES_LDFLAGS_OTHER} ${NOTCURSES_LINK_LIBRARIES})
