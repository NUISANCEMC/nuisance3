#include "Framework/Algorithm/AlgConfigPool.h"
#include "Framework/Algorithm/AlgFactory.h"
#include "Framework/EventGen/XSecAlgorithmI.h"
#include "Framework/Messenger/Messenger.h"
#include "Framework/Registry/Registry.h"
#include "Framework/Utils/RunOpt.h"

#include "RwFramework/GSyst.h"
#include "RwFramework/GSystUncertainty.h"

#include "pybind11/pybind11.h"
#include "pybind11/stl.h"

#include <algorithm>
#include <cctype>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace py = pybind11;

auto TransformPriorityLevelString(std::string priority_value) {

  std::transform(priority_value.begin(), priority_value.end(),
                 priority_value.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  if ((priority_value == "fatal") || (priority_value == "pfatal")) {
    return pFATAL;
  } else if ((priority_value == "alert") || (priority_value == "palert")) {
    return pALERT;
  } else if ((priority_value == "crit") || (priority_value == "pcrit")) {
    return pCRIT;
  } else if ((priority_value == "error") || (priority_value == "perror")) {
    return pERROR;
  } else if ((priority_value == "warn") || (priority_value == "pwarn")) {
    return pWARN;
  } else if ((priority_value == "notice") || (priority_value == "pnotice")) {
    return pNOTICE;
  } else if ((priority_value == "info") || (priority_value == "pinfo")) {
    return pINFO;
  } else if ((priority_value == "debug") || (priority_value == "pdebug")) {
    return pDEBUG;
  }
  std::stringstream ss;
  ss << "Invalid priority_value string: \"" << priority_value << "\".";
  throw std::runtime_error(ss.str());
}

void SetPriorityLevel(std::string const &stream, std::string const &value) {
  //stop the splash screen
  auto default_cout = std::cout.rdbuf();
  std::cout.rdbuf(nullptr);
  auto inst = genie::Messenger::Instance();
  std::cout.rdbuf(default_cout);

  inst->SetPriorityLevel(stream.c_str(), TransformPriorityLevelString(value));
}
void SetPriorityLevel(std::vector<std::string> const &streams,
                      std::string const &value) {
  for (auto const &stream : streams) {
    SetPriorityLevel(stream, value);
  }
}

void RunOpt(std::string tune_name = "", std::string event_generator_list = "") {
  if (!tune_name.size()) {
    if (std::getenv("GENIE_XSEC_TUNE")) {
      tune_name = std::getenv("GENIE_XSEC_TUNE");
    }
    if (!tune_name.size()) {
      throw std::runtime_error(
          "No tune_name passed and GENIE_XSEC_TUNE environment "
          "variable was not valid");
    }
  }

  if (!event_generator_list.size()) {
    if (std::getenv("GENIE_XSEC_EVENTGENERATORLIST")) {
      event_generator_list = std::getenv("GENIE_XSEC_EVENTGENERATORLIST");
    }
    if (!event_generator_list.size()) {
      event_generator_list = "Default";
    }
  }

  genie::RunOpt::Instance()->SetTuneName(tune_name);
  genie::RunOpt::Instance()->BuildTune();
  genie::RunOpt::Instance()->SetEventGeneratorList(event_generator_list);
}

auto XSecAlgorithmParameters(std::string const &xsec_alg_name) {
  py::dict params;

  static bool first = true;
  if (first) {
    SetPriorityLevel(
        std::vector<std::string>{"Nieves", "TransverseEnhancementFFModel"},
        "ERROR");
    first = false;
  }

  if (!genie::RunOpt::Instance()->Tune()) {
    RunOpt();
  }

  auto xsec_alg =
      genie::AlgConfigPool::Instance()->GlobalParameterList()->GetAlg(
          xsec_alg_name);

  auto xsec_alg_model = std::dynamic_pointer_cast<genie::XSecAlgorithmI>(
      std::shared_ptr<genie::Algorithm>(
          genie::AlgFactory::Instance()->AdoptAlgorithm(
              genie::AlgId(xsec_alg))));

  if (!xsec_alg_model) {
    throw std::runtime_error("invalid algorithm name");
  }

  xsec_alg_model->AdoptSubstructure();

  auto xsec_alg_cfg = xsec_alg_model->GetConfig();

  for (auto kv : xsec_alg_cfg.GetItemMap()) {
    auto p_type = xsec_alg_cfg.ItemType(kv.first);
    switch (p_type) {
    case genie::RgType_t::kRgBool: {
      params[kv.first.c_str()] = py::cast(xsec_alg_cfg.GetBool(kv.first));
      break;
    }
    case genie::RgType_t::kRgInt: {
      params[kv.first.c_str()] = py::cast(xsec_alg_cfg.GetInt(kv.first));
      break;
    }
    case genie::RgType_t::kRgDbl: {
      params[kv.first.c_str()] = py::cast(xsec_alg_cfg.GetDouble(kv.first));
      break;
    }
    case genie::RgType_t::kRgStr: {
      params[kv.first.c_str()] = py::cast(xsec_alg_cfg.GetString(kv.first));
      break;
    }
    default: {

      params[kv.first.c_str()] = py::cast(genie::RgType::AsString(p_type));
      break;
    }
    }
  }
  return params;
}

auto OneSigmaErr(std::string const &parameter) {

  static bool first = true;
  if (first) {
    SetPriorityLevel("ReW", "ERROR");
    first = false;
  }

  if (!genie::RunOpt::Instance()->Tune()) {
    RunOpt();
  }
  return genie::rew::GSystUncertainty::Instance()->OneSigmaErr(
      genie::rew::GSyst::FromString(parameter));
}

PYBIND11_MODULE(pyNUISANCEGENIE, m) {

  m.doc() = "NUISANCE GENIE python helpers";
  auto g = m.def_submodule("GENIE");

  g.def("SetPriorityLevel",
        py::overload_cast<std::string const &, std::string const &>(
            &SetPriorityLevel))
      .def("SetPriorityLevel",
           py::overload_cast<std::vector<std::string> const &,
                             std::string const &>(&SetPriorityLevel))
      .def("OneSigmaErr", &OneSigmaErr)
      .def("RunOpt", &RunOpt, py::arg("tune_name") = "",
           py::arg("event_generator_list") = "")
      .def("XSecAlgorithms",
           []() {
             std::vector<std::string> algs;
             if (!genie::RunOpt::Instance()->Tune()) {
               RunOpt();
             }
             for (auto kv : genie::AlgConfigPool::Instance()
                                ->GlobalParameterList()
                                ->GetItemMap()) {
               if (kv.second->TypeInfo() == genie::RgType_t::kRgAlg) {
                 algs.push_back(kv.first);
               }
             }
             return algs;
           })
      .def("XSecAlgorithmParameters", &XSecAlgorithmParameters)
      .def("ParameterValueToTwk",
           [](std::pair<std::string, std::string> const &param_path,
              std::string const &reweight_dial, double value) {
             auto params = XSecAlgorithmParameters(param_path.first);
             double defval = params[param_path.second.c_str()].cast<double>();
             double uncert = OneSigmaErr(reweight_dial);

             // val = def * (1 + twk * uncert)
             return ((value / defval) - 1) / uncert;
           });
}
