set(UNIDAMAGE_SOURCES
  unidamage.cc
  )

add_executable(ncpp-unidamage ${UNIDAMAGE_SOURCES})
add_dependencies(ncpp-unidamage notcurses++)
target_link_libraries(ncpp-unidamage PRIVATE notcurses++)
install(TARGETS ncpp-unidamage DESTINATION bin)

set(RGB_SOURCES
  rgb.cc
  )

add_executable(ncpp-rgb ${RGB_SOURCES})
add_dependencies(ncpp-rgb notcurses++)
target_link_libraries(ncpp-rgb PRIVATE notcurses++)
install(TARGETS ncpp-rgb DESTINATION bin)

set(RGB_SOURCES
  rgbbg.cc
  )

add_executable(ncpp-rgbbg ${RGB_SOURCES})
add_dependencies(ncpp-rgbbg notcurses++)
target_link_libraries(ncpp-rgbbg PRIVATE notcurses++)
install(TARGETS ncpp-rgbbg DESTINATION bin)
