set(DEMO_SOURCES
  boxdemo.cc
  chunli.cc
  demo.cc
  eagle.cc
  grid.cc
  hud.cc
  input.cc
  luigi.cc
  outro.cc
  panelreel.cc
  sliding.cc
  trans.cc
  unicodeblocks.cc
  view.cc
  witherworm.cc
  xray.cc
  )

add_executable(demo ${DEMO_SOURCES})
add_dependencies(demo notcurses++)
target_link_libraries(demo PRIVATE notcurses++)
