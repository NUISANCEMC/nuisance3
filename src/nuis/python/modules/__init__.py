from ._pyNUISANCE import *

try:
  import requests
  from .HepDataRefResolver import GetLocalPathToResource
  import os
  import types
  import yaml
  import pyProSelecta as pps

  hepdata = types.SimpleNamespace()

  def _get_local_path_to_resource(ref, *args, **context):

    record_database_root = os.environ.get("NUISANCE_RECORD_DATABASE")

    if not record_database_root:
      raise RuntimeError("NUISANCE_RECORD_DATABASE environment variable is not defined")

    return GetLocalPathToResource(record_database_root, ref, **context)

  def _get_independent_variables(table):
    return [ x["header"]["name"] for x in table["independent_variables"] ]
  
  def _get_dependent_variables(table):
    return [ x["header"]["name"] for x in table["dependent_variables"] ]

  def _get_qualifiers(table, dependent_variable=0):
    if isinstance(dependent_variable, str):
      dependent_variable = _get_dependent_variables(table).index(dependent_variable)
    return { kv["name"]:kv["value"] for kv in table["dependent_variables"][dependent_variable]["qualifiers"] }

  def _get_tables(submission_path):
    tables = {}

    with open("/".join([submission_path, "submission.yaml"]), 'r') as file:
      submission_yaml = yaml.safe_load_all(file)
    
      # loop through sub-documents in the submission file and load up all referenced tables
      for doc in submission_yaml:
          if "data_file" in doc:
              df = doc["data_file"]
              with open("/".join([submission_path, df]), 'r') as datafile:
                  tables[df[0:-5]] = yaml.safe_load(datafile)
    return tables

  def _get_selection_function(table, dependent_variable=0):
    quals = _get_qualifiers(table, dependent_variable)
    return (quals["select"], pps.select.get(quals["select"]))

  def _get_projection_functions(table, dependent_variable=0):
    quals = _get_qualifiers(table, dependent_variable)
    project_names = [ quals["project:%s" % v] for v in _get_independent_variables(table) ]

    return (project_names, [ pps.project.get(fn) for fn in project_names ])

  def _make_BinnedValues(table):
    return convert.HistFrame.from_yaml_str(str(table))

  def _get_referenced_table(ref, *args, **context):
    local_path = _get_local_path_to_resource(ref, **context)
    with open(local_path) as datafile:
      return yaml.safe_load(datafile)
    raise RuntimeError("Failed to parse %s at yaml document" % local_path)

  def _get_sub_measurement_tables(table, dependent_variable=0, *args, **context):
    quals = _get_qualifiers(table, dependent_variable)
    return [ _get_referenced_table(m, **context) for m in quals["sub_measurements"].split(",") ]

  def _get_covariance(table, dependent_variable=0, *args, **context):
    quals = _get_qualifiers(table, dependent_variable)
    local_path = _get_local_path_to_resource(quals["covariance"], **context)
    with open(local_path) as datafile:
      return convert.Covariance.from_yaml_str(str(yaml.safe_load(datafile)))

  hepdata.get_local_path_to_resource = _get_local_path_to_resource
  hepdata.get_independent_variables = _get_independent_variables
  hepdata.get_dependent_variables = _get_dependent_variables
  hepdata.get_selection_function = _get_selection_function
  hepdata.get_projection_functions = _get_projection_functions
  hepdata.get_qualifiers = _get_qualifiers
  hepdata.get_sub_measurement_tables = _get_sub_measurement_tables
  hepdata.get_covariance = _get_covariance
  hepdata.get_tables = _get_tables
  hepdata.get_referenced_table = _get_referenced_table
  hepdata.make_BinnedValues = _make_BinnedValues

except ModuleNotFoundError:
    pass

# Matplotlib interfaces
try:
    import matplotlib.pyplot as plt
    import matplotlib as mpl
    from .matplotlib_helpers import *
except ModuleNotFoundError:
    pass


# try:
#   import ROOT
#   from .root_helpers import *
# except ModuleNotFoundError:
#     pass
