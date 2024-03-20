add_library(hepdata_record_plugin SHARED HEPDATARecord.cxx)
target_link_libraries(hepdata_record_plugin PUBLIC histframe nuis_options)

set_target_properties(hepdata_record_plugin PROPERTIES PREFIX "nuisplugin-record-")
set_target_properties(hepdata_record_plugin PROPERTIES OUTPUT_NAME "hepdata")

install(TARGETS hepdata_record_plugin DESTINATION lib/plugins)


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

  install(TARGETS nuisance2_record_plugin DESTINATION lib/plugins)
endif()