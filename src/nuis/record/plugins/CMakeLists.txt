add_library(hepdata_record_plugin SHARED HEPDATARecord.cxx)
target_link_libraries(hepdata_record_plugin PUBLIC nuis_options)

set_target_properties(hepdata_record_plugin PROPERTIES PREFIX "nuisplugin-record-")
set_target_properties(hepdata_record_plugin PROPERTIES OUTPUT_NAME "hepdata")

install(TARGETS hepdata_record_plugin DESTINATION lib/plugins)


#add_library(nuisance_record_plugin SHARED NUISANCERecord.cxx)
#target_link_libraries(nuisance_record_plugin PUBLIC nuis_options)

#set_target_properties(nuisance_record_plugin PROPERTIES PREFIX "nuisplugin-record-")
#set_target_properties(nuisance_record_plugin PROPERTIES OUTPUT_NAME "nuisance")

#install(TARGETS nuisance_record_plugin DESTINATION lib/plugins)