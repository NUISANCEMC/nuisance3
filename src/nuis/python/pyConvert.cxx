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
      .def("to_plotly1D", &to_plotly1D)
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
      .def(
          "DumpTH1s",
          [](std::string const &foutname,
             std::vector<std::pair<
                 std::string, std::reference_wrapper<BinnedValues const>>> const
                 &hfs) {
            TFile fout(foutname.c_str(), "RECREATE");
            for (auto const &[hname, hf] : hfs) {
              auto h = ToTH1(hf, hname, false);
              fout.WriteTObject(h.get(), hname.c_str());
              h->SetDirectory(nullptr);
            }
            fout.Write();
            fout.Close();
          });
}
