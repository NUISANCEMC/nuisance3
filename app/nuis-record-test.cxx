#include "nuis/eventinput/EventSourceFactory.h"

#include "nuis/weightcalc/WeightCalcFactory.h"
#include "indicators.hpp"

#include "nuis/frame/FrameGen.h"
#include "nuis/histframe/HistFrame.h"
#include "nuis/histframe/ROOTUtility.h"
#include "nuis/histframe/Utility.h"

#include "nuis/record/RecordFactory.h"


#include "NuHepMC/EventUtils.hxx"
#include "NuHepMC/FATXUtils.hxx"
#include "NuHepMC/ReaderUtils.hxx"

#include "TCanvas.h"

#include "spdlog/spdlog.h"

#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wnarrowing"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

using namespace nuis;

int main(int argc, char const *argv[]) {

  EventSourceFactory fact;
  auto [gri, evs] = fact.make(argv[1]);

  RecordFactory rfact;
  YAML::Node cfg;
  cfg["type"] = "hepdata";
  cfg["release"] = "ANL/CCQE/182176/";
  cfg["table"] = "EventCounts-Q2";

  // Full Individual Processing
  auto anl_record        = rfact.make(cfg);
  auto anl_table         = anl_record->table("EventCounts-Q2");
  auto anl_compare_neut  = anl_table->comparison();
  auto anl_compare_nuwro = anl_table->comparison();

  int count = 0;
  for (auto const &[ev, cvw] : evs) {

    anl_compare_neut.fill_with_selection(
      anl_table->select(ev),
      anl_table->project(ev),
      cvw * anl_table->weight(ev));

    count++;
    if (count % 10000 == 0) std::cout << count << std::endl;
    if (count > 10000) break;
  }

  std::cout << "Raw" << std::endl;
  std::cout << anl_compare_neut["data"].getcv() << std::endl;
  std::cout << anl_compare_neut["mc"].getcv() << std::endl;

  auto norm_info = evs->norm_info();
  anl_table->finalize(anl_compare_neut,
    (norm_info.fatx / norm_info.sumweights) );

  std::cout << "Finalized" << std::endl;
  std::cout << anl_compare_neut["data"].getcv() << std::endl;
  std::cout << anl_compare_neut["mc"].getcv() << std::endl;

  // Inline Individual Processing (doesn't work due to ProSel conflict issue)
  // auto compare_neut2  = rfact.make(cfg)->table("EventRate-Q2")->comparison();
  // auto compare_nuwro2 = rfact.make(cfg)->table("EventRate-Q2")->comparison();
  // std::cout << compare_nuwro2["data"].getcv() << std::endl;

}