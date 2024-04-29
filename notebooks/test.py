import pyNUISANCE as pn
import numpy as np
import pyProSelecta as pps
import pyNuHepMC as nhm
import matplotlib.pyplot as plt
import scipy

recordid=1713531371
submission_path, resource_path, reference = pn.hpd.get_local_path_to_resource(reftype="hepdata-sandbox", recordid=recordid)

onax_doc = pn.hpd.get_table("cross_section-onaxis",**reference)
offax_doc = pn.hpd.get_table("cross_section-offaxis",**reference)

onax_data = pn.convert.HistFrame.from_yaml_str(str(onax_doc))
offax_data = pn.convert.HistFrame.from_yaml_str(str(offax_doc))

x,y,c = pn.convert.HistFrame.to_mpl_pcolormesh(offax_data.slice(0,[0,5]),0)
