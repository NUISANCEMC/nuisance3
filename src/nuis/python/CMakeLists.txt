pybind11_add_module(pyEventInput SHARED pyEventInput.cxx)

target_link_libraries(pyEventInput PRIVATE eventinput nuis_options NuHepMC::CPPUtils pybind11::module HepMC3::All)

target_include_directories(pyEventInput PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/..>
  ${Python3_INCLUDE_DIRS})

install(TARGETS pyEventInput
    LIBRARY DESTINATION ${NUISANCE_PYTHONPATH})
