import sys
import copy
import os

import pyNUISANCE as pn

import yaml

out_file = sys.argv[1]
histogram_name = sys.argv[2]
flux_ref = sys.argv[3]

if os.path.exists(flux_ref):
  resource_path = flux_ref
  ncontext = {}
else:
  submission_path, resource_path, ncontext = pn.hpd.get_local_path_to_resource(flux_ref, reftype="hepdata")

with open(resource_path,'r') as yfile:
  flux_table = yaml.safe_load(yfile)

dependent_variable = None

if "qualifier" in ncontext:
  dependent_variable = ncontext["qualifier"]

dependent_variable_idx = 0
if dependent_variable:
  dependent_variable_idx = get_dependent_variables(ref, **ncontext).index(dependent_variable)

qualifiers = { kv["name"]:kv["value"] for kv in flux_table["dependent_variables"][dependent_variable_idx]["qualifiers"] }

content_type = "count_per_width"
if "bin_content_type" in qualifiers:
  content_type = qualifiers["bin_content_type"]

hf_per_width = pn.convert.HistFrame.from_yaml_str(str(flux_table))
hf_count = copy.copy(hf_per_width)

if content_type == "count_per_width":
  vals = hf_count.values.copy()
  errs = hf_count.errors.copy()
  rows,cols = vals.shape

  for c in range(cols):
    vals[:,c] = vals[:,c] * hf_count.binning.bin_sizes()
    errs[:,c] = errs[:,c] * hf_count.binning.bin_sizes()

  hf_count.values = vals.copy()
  hf_count.errors = errs.copy()
  hf_count.column_info[0].column_label="A.U."

elif content_type == "count":
  vals = hf_per_width.values.copy()
  errs = hf_per_width.errors.copy()
  rows,cols = vals.shape

  for c in range(cols):
    vals[:,c] = vals[:,c] / hf_per_width.binning.bin_sizes()
    errs[:,c] = errs[:,c] / hf_per_width.binning.bin_sizes()

  hf_per_width.values = vals.copy()
  hf_per_width.errors = errs.copy()
  hf_per_width.column_info[0].column_label="A.U."
else:
  raise RuntimeException("Unknown flux histogram content type: %s" % content_type)

pn.convert.ROOT.write_TH1(out_file, "%s_count" % histogram_name,hf_count,False,"RECREATE")
pn.convert.ROOT.write_TH1(out_file, "%s_count_per_width" % histogram_name,hf_per_width,False,"UPDATE")