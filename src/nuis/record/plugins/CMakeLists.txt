add_library(record_plugins INTERFACE)

add_library(hepdata_record_plugin SHARED HEPDataRecord.cxx)
target_link_libraries(hepdata_record_plugin PUBLIC histframe nuis_options)

set_target_properties(hepdata_record_plugin PROPERTIES PREFIX "nuisplugin-record-")
set_target_properties(hepdata_record_plugin PROPERTIES OUTPUT_NAME "hepdata")
target_compile_definitions(hepdata_record_plugin PUBLIC NUIS_RECORD_HEPData_Enabled)

install(TARGETS hepdata_record_plugin DESTINATION ${NUISANCE_PLUGIN_LIBDIR})

target_link_libraries(record_plugins INTERFACE hepdata_record_plugin)


if(NUISANCE_LEGACY_INTERFACE)
  if(NOT DEFINED ENV{NUISANCE})
    message(FATAL_ERROR "NUISANCE_LEGACY_INTERFACE: ${NUISANCE_LEGACY_INTERFACE} is enabled but ENV{NUISANCE} is not defined.")
  endif()

  LIST(APPEND CMAKE_PREFIX_PATH $ENV{NUISANCE})
  find_package(NUISANCE REQUIRED)

  add_library(nuisance2_record_plugin SHARED NUISANCE2Record.cxx)
  target_link_libraries(nuisance2_record_plugin PUBLIC histframe NUIS::FCN)

  set_target_properties(nuisance2_record_plugin PROPERTIES PREFIX "nuisplugin-record-")
  set_target_properties(nuisance2_record_plugin PROPERTIES OUTPUT_NAME "nuisance2")
  target_compile_definitions(nuisance2_record_plugin PUBLIC NUIS_RECORD_NUISANCE2_Enabled)

  target_link_libraries(record_plugins INTERFACE nuisance2_record_plugin)

  install(TARGETS nuisance2_record_plugin DESTINATION ${NUISANCE_PLUGIN_LIBDIR})
endif()