#include "nuis/eventinput/IEventSource.h"

#include "nuwroconv.h"

#include "HepMC3/GenRunInfo.h"

#include "TChain.h"
#include "TFile.h"

#include "boost/dll/alias.hpp"

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/sum_kahan.hpp>

using namespace boost::accumulators;

#include "spdlog/spdlog.h"

#include "yaml-cpp/yaml.h"

#include <filesystem>
#include <fstream>

namespace nuis {

class NuWroevent1EventSource : public IEventSource {

  std::vector<std::filesystem::path> filepaths;
  std::unique_ptr<TChain> chin;

  std::shared_ptr<HepMC3::GenRunInfo> gri;

  Long64_t ch_ents;
  Long64_t ient;
  TUUID ch_fuid;

  event *ev;

  accumulator_set<double, stats<tag::sum_kahan>> sumw;

  void CheckAndAddPath(std::filesystem::path filepath) {
    if (!std::filesystem::exists(filepath)) {
      spdlog::warn("NuWroevent1EventSource ignoring non-existant path {}",
                   filepath.native());
      return;
    }
    std::ifstream fin(filepath);
    char magicbytes[5];
    fin.read(magicbytes, 4);
    magicbytes[4] = '\0';
    if (std::string(magicbytes) != "root") {
      spdlog::warn(
          "NuWroevent1EventSource ignoring non-root file {} (magicbytes: {})",
          filepath.native(), magicbytes);
      return;
    }
    filepaths.push_back(std::move(filepath));
  }

public:
  NuWroevent1EventSource(YAML::Node const &cfg) {
    if (cfg["filepath"]) {
      CheckAndAddPath(cfg["filepath"].as<std::string>());
    } else if (cfg["filepaths"]) {
      for (auto fp : cfg["filepaths"].as<std::vector<std::string>>()) {
        CheckAndAddPath(fp);
      }
    }
  };

  std::optional<HepMC3::GenEvent> first() {
    //reset the weight counter, can't be bothered to copy out the type
    sumw = decltype(sumw)();
    
    if (!filepaths.size()) {
      return std::optional<HepMC3::GenEvent>();
    }

    chin = std::make_unique<TChain>("treeout");

    for (auto const &ftr : filepaths) {
      if (!chin->Add(ftr.c_str(), 0)) {
        spdlog::warn("Could not find treeout in {}", ftr.native());
        chin.reset();
        return std::optional<HepMC3::GenEvent>();
      }
    }

    ch_ents = chin->GetEntries();
    ev = nullptr;
    auto branch_status = chin->SetBranchAddress("e", &ev);
    chin->GetEntry(0);

    double fatx = ev->weight;

    gri = nuwroconv::BuildRunInfo(ch_ents, fatx, ev->par);

    ch_fuid = chin->GetFile()->GetUUID();
    ient = 0;
    auto ge = nuwroconv::ToGenEvent(*ev, gri);
    ge.set_event_number(ient);
    sumw(ge.weights().front());
    return ge;
  }

  std::optional<HepMC3::GenEvent> next() {
    ient++;

    if (ient >= ch_ents) {
      return std::optional<HepMC3::GenEvent>();
    }

    chin->GetEntry(ient);

    if (chin->GetFile()->GetUUID() != ch_fuid) {
      ch_fuid = chin->GetFile()->GetUUID();
    }

    auto ge = nuwroconv::ToGenEvent(*ev, gri);
    ge.set_event_number(ient);
    sumw(ge.weights().front());
    return ge;
  }

  double sum_weights_so_far() { return sum_kahan(sumw); }

  std::shared_ptr<HepMC3::GenRunInfo> run_info() { return gri; }

  static IEventSourcePtr MakeEventSource(YAML::Node const &cfg) {
    return std::make_shared<NuWroevent1EventSource>(cfg);
  }

  virtual ~NuWroevent1EventSource() {}
};

BOOST_DLL_ALIAS(nuis::NuWroevent1EventSource::MakeEventSource, MakeEventSource);

} // namespace nuis
