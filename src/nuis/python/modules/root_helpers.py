from ROOT import TH1D
from array import array

from ._pyNUISANCE import convert, Binning

def ToTH1(bv, name, column=None):
  if not column: column = bv.column_info[0].name

  ci =  bv.find_column_index(column)

  bin_edges = Binning.get_bin_edges1D(bv.binning.bins)

  hout = TH1D(name,"",len(bin_edges)-1, array('d',bin_edges))

  contents = bv.get_bin_contents()
  uncert = bv.get_bin_uncertainty()

  for bi in range(len(bv.binning.bins)):
    hout.SetBinContent(bi+1, contents[bi,ci])
    hout.SetBinError(bi+1, uncert[bi,ci])

  return hout

convert.ROOT.ToTH1 = ToTH1