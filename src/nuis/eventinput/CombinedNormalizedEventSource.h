#pragma once

#include "nuis/eventinput/NormalizedEventSource.h"

namespace nuis {

class CombinedNormalizedEventSource : public NormalizedEventSource {

  enum CStatus { kFirst, kHasEvents, kFinished };

  std::vector<std::pair<CStatus, NormalizedEventSourcePtr>> comp_evs;
  size_t src_it;

  enum COp { kAdd, kAvg, kNone };
  COp combo_op;

  CombinedNormalizedEventSource(
      CombinedNormalizedEventSource::COp op,
      std::vector<NormalizedEventSourcePtr> components);

  size_t next_src();

public:
  static NormalizedEventSourcePtr
  SumEventSources(std::vector<NormalizedEventSourcePtr> components);

  static NormalizedEventSourcePtr
  AverageEventSources(std::vector<NormalizedEventSourcePtr> components);

  std::optional<EventCVWeightPair> first();
  std::optional<EventCVWeightPair> next();

  NormInfo norm_info(NuHepMC::CrossSection::Units::Unit const &units);
  virtual ~CombinedNormalizedEventSource();
};

} // namespace nuis