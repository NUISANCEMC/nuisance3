add_library(eventinput SHARED IEventSourceIterator.cxx)

target_link_libraries(eventinput PUBLIC nuis_options)

add_subdirectory(plugins)

install(TARGETS eventinput DESTINATION lib)

pybind11_add_module(pyEVENTINPUT SHARED pyEVENTINPUT.cxx)

target_link_libraries(pyEVENTINPUT PRIVATE pybind11::embed eventinput nuis_options NuHepMC::CPPUtils HepMC3::All)

target_include_directories(pyEVENTINPUT PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/..>
  ${Python3_INCLUDE_DIRS})

install(TARGETS pyEVENTINPUT
    LIBRARY DESTINATION ${NUISANCE_PYTHONPATH}/)
