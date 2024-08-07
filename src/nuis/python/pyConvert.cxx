#include "nuis/convert/ROOT.h"
#include "nuis/convert/misc.h"
#include "nuis/convert/yaml.h"

#include "nuis/python/pyNUISANCE.h"

#include "nuis/log.txx"

#include "pybind11/eigen.h"

namespace py = pybind11;
using namespace nuis;

void pyConvertInit(py::module &m) {
  auto convmod = m.def_submodule("convert", "");
  convmod.def_submodule("HistFrame", "")
      .def("to_mpl_pcolormesh", &to_mpl_pcolormesh)
      .def("to_yaml_str", &to_yaml_str)
      .def("from_yaml_str", &from_yaml_str);
  convmod.def_submodule("Covariance", "")
      .def("from_yaml", &covar_from_yaml)
      .def("from_yaml_str", &covar_from_yaml_str);
  convmod.def_submodule("ROOT", "")
      .def("get_EnergyDistribution_from_ROOT",
           &get_EnergyDistribution_from_ROOT, py::arg("fname"),
           py::arg("hname"), py::arg("energy_unit") = std::string(""),
           py::arg("is_per_width") = false)
      .def("write_TH1", [](std::string const &foutname,
                           std::string const &hname, BinnedValues const &hf,
                           bool divide_by_bin_width, std::string const &opts) {
        TFile fout(foutname.c_str(), opts.c_str());
        auto h = ToTH1(hf, hname, divide_by_bin_width);
        fout.WriteTObject(h.get(), hname.c_str());
        h->SetDirectory(nullptr);
        fout.Write();
        fout.Close();
      });
}
