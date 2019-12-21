set(VIEW_SOURCES
  view.cc
  )

add_executable(view ${VIEW_SOURCES})
add_dependencies(view notcurses++)
target_link_libraries(view PRIVATE notcurses++ ${AVUTIL_LDFLAGS} ${AVUTIL_LDFLAGS_OTHER} ${AVUTIL_LINK_LIBRARIES})
