install(PROGRAMS
  nuis
  nuis-flux
  nuis-gen
  nuis-internal
  nuis-hepdata
  DESTINATION bin)

# any internal nasty details can 
# be kept in scripts in here, these 
# will never form part of the public CLI
install(DIRECTORY
  internal
  DESTINATION var)