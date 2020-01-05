set(NCPP_SOURCES
  NotCurses.cc
  PanelReel.cc
  Plane.cc
  Root.cc
  Tablet.cc
  Visual.cc
  )

add_library(
  notcurses++
  SHARED
  ${NCPP_SOURCES}
  )

add_library(
  notcurses++-static
  STATIC
  ${NCPP_SOURCES}
  )
set_target_properties(notcurses++-static PROPERTIES
  OUTPUT_NAME notcurses++
  )

if(APPLE)
  set_target_properties(notcurses++ PROPERTIES
    MACOSX_RPATH 1
    OUTPUT_NAME "notcurses++-${LT_RELEASE}")
elseif(UNIX)
  set_target_properties(notcurses++ PROPERTIES
    VERSION ${LT_VERSION}
    SOVERSION ${LT_REVISION}
    OUTPUT_NAME "notcurses++")
else()                                                                                                                                                                                                                                                                                                   
  message(FATAL_ERROR "Only Unix supported, sorry")
endif()

target_link_libraries(notcurses++ ${NOTCURSES_LDFLAGS} ${NOTCURSES_LDFLAGS_OTHER} ${NOTCURSES_LINK_LIBRARIES})

file(GLOB NCPP_HEADERS
  CONFIGURE_DEPENDS
  LIST_DIRECTORIES false
  ${CMAKE_SOURCE_DIR}/include/ncpp/*.hh)

file(GLOB NCPP_INTERNAL_HEADERS
  CONFIGURE_DEPENDS
  LIST_DIRECTORIES false
  ${CMAKE_SOURCE_DIR}/include/ncpp/internal/*.hh)

install(FILES ${NCPP_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ncpp)
install(FILES ${NCPP_INTERNAL_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ncpp/internal)

install(
  TARGETS notcurses++
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT Libraries
    NAMELINK_COMPONENT Development
)

install(
  TARGETS notcurses++-static
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT Libraries
    NAMELINK_COMPONENT Development
)
