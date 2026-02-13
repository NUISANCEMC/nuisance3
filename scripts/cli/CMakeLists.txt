install(PROGRAMS
  nuis
  nuis-flux
  nuis-gen
  nuis-internal
  DESTINATION bin)

install(DIRECTORY
# any internal nasty details can 
# be kept in scripts in here, these 
# will never form part of the public CLI
  internal
  funcs
  DESTINATION var)