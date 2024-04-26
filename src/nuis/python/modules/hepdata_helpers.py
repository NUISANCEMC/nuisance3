import os
import types

import yaml

from .HepDataRefResolver import GetLocalPathToResource, ResolveReferenceIdentifiers

def get_local_path_to_resource(ref, **context):

  record_database_root = os.environ.get("NUISANCE_RECORD_DATABASE")

  if not record_database_root:
    raise RuntimeError("NUISANCE_RECORD_DATABASE environment variable is not defined")

  return GetLocalPathToResource(record_database_root, ref, **context)

def context_to_ref(**context):
  record_ref = context["reftype"] + ":" + context["recordid"]

  if context["recordvers"]:
   record_ref = record_ref + "v" + context["recordvers"]

  if context["resourcename"]:
    record_ref = record_ref + "/" + context["resourcename"]
    if context["qualifier"]:
      record_ref = record_ref + ":" + context["qualifier"]

  return record_ref


def get_table(ref, **context):
  submission_path, resource_path, ncontext = get_local_path_to_resource(ref, **context)

  if not ncontext["resourcename"]:
    raise RuntimeError("pyNUISANCE.hpd.get_table called with reference %s and context %s that doesn't resolve to a specific table" % (ref,context))

  with open(resource_path, 'r') as file:
    return yaml.safe_load(file)

def get_independent_variables(ref, **context):
  return [ x["header"]["name"] for x in get_table(ref, **context)["independent_variables"] ]

def get_dependent_variables(ref, **context):
  return [ x["header"]["name"] for x in get_table(ref, **context)["dependent_variables"] ]

def get_qualifiers(ref, **context):
  submission_path, resource_path, ncontext = get_local_path_to_resource(ref, **context)

  table = get_table(ref, **ncontext)
  dv = ncontext["qualifier"]

  dv_idx = 0
  if dv:
    dv_idx = get_dependent_variables(ref, **ncontext).index(dv)

  return { kv["name"]:kv["value"] for kv in table["dependent_variables"][dv_idx]["qualifiers"] }

def get_table_names(ref, **context):

  submission_path, resource_path, ncontext = get_local_path_to_resource(ref, **context)

  tables = []

  with open("/".join([submission_path, "submission.yaml"]), 'r') as file:
    submission_yaml = yaml.safe_load_all(file)
    # loop through sub-documents in the submission file and list all named tables
    for doc in submission_yaml:
      if "data_file" in doc:
        tables.append(doc["data_file"][0:-5])

  return tables

def get_local_additional_resources(ref, **context):
  submission_path, resource_path, ncontext = get_local_path_to_resource(ref, **context)

  addres = {}

  with open("/".join([submission_path, "submission.yaml"]), 'r') as file:
    submission_yaml = yaml.safe_load_all(file)
    # loop through sub-documents in the submission file and list all named tables
    for doc in submission_yaml:

      if not "additional_resources" in doc:
        continue

      docres = []
      docname = "common"

      if "data_file" in doc:
        docname = doc["name"]

      for ar in doc["additional_resources"]:
        if os.path.exists("/".join([submission_path, ar["location"]])):
          docres.append(ar["location"])

      if len(docres):
        addres[docname] = docres

  return addres

hepdata = types.SimpleNamespace()

hepdata.GetLocalPathToResource = GetLocalPathToResource
hepdata.ResolveReferenceIdentifiers = ResolveReferenceIdentifiers

hepdata.get_local_path_to_resource = get_local_path_to_resource
hepdata.context_to_ref = context_to_ref
hepdata.get_table = get_table
hepdata.get_independent_variables = get_independent_variables
hepdata.get_dependent_variables = get_dependent_variables
hepdata.get_qualifiers = get_qualifiers
hepdata.get_table_names = get_table_names
hepdata.get_local_additional_resources = get_local_additional_resources

