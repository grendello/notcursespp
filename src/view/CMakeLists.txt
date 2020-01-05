if(NOT DISABLE_FFMPEG)
  set(VIEW_SOURCES
	view.cc
	)

  add_executable(ncpp-view ${VIEW_SOURCES})
  add_dependencies(ncpp-view notcurses++)
  target_link_libraries(ncpp-view PRIVATE notcurses++ ${AVUTIL_LDFLAGS} ${AVUTIL_LDFLAGS_OTHER} ${AVUTIL_LINK_LIBRARIES})

  install(TARGETS ncpp-view DESTINATION bin)
endif()
