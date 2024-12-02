add_subdirectory(eventinput)

add_subdirectory(eventframe)

add_subdirectory(binning)
add_subdirectory(histframe)

add_subdirectory(weightcalc)

if(NUISANCE_USE_HEPDATA)
  add_subdirectory(record)
endif()

add_subdirectory(convert)
add_subdirectory(response)

add_subdirectory(python)