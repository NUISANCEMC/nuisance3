import os
import types

import yaml

from .HepDataRefResolver import GetLocalPathToResource, ResolveReferenceIdentifiers

def context_to_ref(**context):
  record_ref = context["reftype"] + ":" + context["recordid"]

  if context["recordvers"]:
   record_ref = record_ref + "v" + context["recordvers"]

  if context["resourcename"]:
    record_ref = record_ref + "/" + context["resourcename"]
    if context["qualifier"]:
      record_ref = record_ref + ":" + context["qualifier"]

  return record_ref

def get_local_path_to_resource(ref="", **context):

  if ':' in ref and os.path.exists(ref.split(':')[0]):
    return "", ref.split(':')[0], {"qualifier": ref.split(':')[1], "resourcename": os.path.basename(ref.split(':')[0])}

  if ref and os.path.exists(ref):
    return "", ref, {"resourcename": os.path.basename(ref.split(':')[0])}

  record_database_root = os.environ.get("NUISANCE_RECORD_DATABASE")

  if not record_database_root:
    raise RuntimeError("NUISANCE_RECORD_DATABASE environment variable is not defined")


  rctx = ResolveReferenceIdentifiers(ref, **context)

  if rctx.get("reftype") == "nuis":
    hepdata_map = "/".join([record_database_root,"nuis","nuis2hepdata.yaml"])
    with open(hepdata_map, 'r') as mapfile:
      nuis2hepdata = yaml.safe_load(mapfile)

    try:
      hepdata_key = nuis2hepdata[rctx["recordid"]][rctx["resourcename"]][rctx["qualifier"]]
    except KeyError:
      print("[ERROR] Could not resolve reference %s. Check %s." %(ref,hepdata_map))

    return GetLocalPathToResource(record_database_root, hepdata_key)

  return GetLocalPathToResource(record_database_root, ref, **context)

def get_table(ref="", **context):
  submission_path, resource_path, ncontext = get_local_path_to_resource(ref, **context)

  if not ncontext["resourcename"]:
    raise RuntimeError("pyNUISANCE.hpd.get_table called with reference %s and context %s that doesn't resolve to a specific table" % (ref,context))

  with open(resource_path, 'r') as file:
    return yaml.safe_load(file)

def get_independent_variables(ref="", **context):
  return [ x["header"]["name"] for x in get_table(ref, **context)["independent_variables"] ]

def get_dependent_variables(ref="", **context):
  return [ x["header"]["name"] for x in get_table(ref, **context)["dependent_variables"] ]

def get_qualifiers(ref="", **context):
  submission_path, resource_path, ncontext = get_local_path_to_resource(ref, **context)

  table = get_table(ref, **ncontext)
  dv = ncontext.get("qualifier")

  dv_idx = 0
  if dv:
    dv_idx = get_dependent_variables(ref, **ncontext).index(dv)

  return { kv["name"]:kv["value"] for kv in table["dependent_variables"][dv_idx]["qualifiers"] }

def _resolve_possibly_directly_referenced_submission_file(submission_path, resource_path):
  if not submission_path and (os.path.basename(resource_path) == "submission.yaml"):
    return os.path.dirname(resource_path), resource_path
  return submission_path, resource_path

def get_table_names(ref="", **context):

  submission_path, resource_path, ncontext = get_local_path_to_resource(ref, **context)
  submission_path, resource_path = _resolve_possibly_directly_referenced_submission_file(submission_path, resource_path)

  tables = []

  with open("/".join([submission_path, "submission.yaml"]), 'r') as file:
    submission_yaml = yaml.safe_load_all(file)
    # loop through sub-documents in the submission file and list all named tables
    for doc in submission_yaml:
      if "data_file" in doc:
        tables.append(doc["data_file"][0:-5])

  return tables

def get_local_additional_resources(ref="", **context):
  submission_path, resource_path, ncontext = get_local_path_to_resource(ref, **context)
  submission_path, resource_path = _resolve_possibly_directly_referenced_submission_file(submission_path, resource_path)

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

