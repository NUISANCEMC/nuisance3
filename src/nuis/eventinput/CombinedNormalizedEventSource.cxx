#include "nuis/eventinput/CombinedNormalizedEventSource.h"

#include "nuis/log.txx"
#include "nuis/except.h"

#include <numeric>

namespace nuis {

DECLARE_NUISANCE_EXCEPT(CombinedWithSelf);

CombinedNormalizedEventSource::CombinedNormalizedEventSource(
    CombinedNormalizedEventSource::COp op,
    std::vector<NormalizedEventSourcePtr> components)
    : NormalizedEventSource(nullptr), src_it{0}, combo_op{op} {
  for (auto &c : components) {
    for (auto &[_, pc] : comp_evs) {
      if (pc.get() == c.get()) {
        log_critical("[CombinedNormalizedEventSource] Tried to combine the "
                     "same NormalizedEventSource with itself.");
        throw CombinedWithSelf();
      }
    }
    comp_evs.push_back(std::pair<CStatus, NormalizedEventSourcePtr>(kFirst, c));
  }
}

NormalizedEventSourcePtr CombinedNormalizedEventSource::SumEventSources(
    std::vector<NormalizedEventSourcePtr> components) {
  return std::shared_ptr<CombinedNormalizedEventSource>(
      new CombinedNormalizedEventSource(kAdd, components));
}

NormalizedEventSourcePtr CombinedNormalizedEventSource::AverageEventSources(
    std::vector<NormalizedEventSourcePtr> components) {
  return std::shared_ptr<CombinedNormalizedEventSource>(
      new CombinedNormalizedEventSource(kAvg, components));
}

size_t CombinedNormalizedEventSource::next_src() {
  size_t next = (src_it + 1) % comp_evs.size();

  for (; next < comp_evs.size(); next++) {
    if (comp_evs[next].first != kFinished) {
      return next;
    }
  }
  return next;
}

std::optional<EventCVWeightPair> CombinedNormalizedEventSource::first() {

  NUIS_LOG_TRACE(
      "[CombinedNormalizedEventSource] first(), {} with {} components.",
      (combo_op == kAdd ? "summing" : "averaging"), comp_evs.size());

  if (!comp_evs.size()) {
    return std::optional<EventCVWeightPair>();
  }

  src_it = 0;

  for (auto &[status, evs] : comp_evs) {
    status = kFirst;
  }

  auto ev = comp_evs[src_it].second->first();

  if (!ev) {
    log_error(
        "[CombinedNormalizedEventSource]: First component EventSource couldn't "
        "get the first event.");
    comp_evs[src_it].first = kFinished;
  } else {
    comp_evs[src_it].first = kHasEvents;
  }

  return ev;
}

std::optional<EventCVWeightPair> CombinedNormalizedEventSource::next() {

  src_it = next_src();

  if (src_it == comp_evs.size()) { // all sources empty
    return std::optional<EventCVWeightPair>();
  }

  std::optional<EventCVWeightPair> ev;

  while (!ev && (src_it != comp_evs.size())) {

    switch (comp_evs[src_it].first) {
    case kFirst: {
      ev = comp_evs[src_it].second->first();
      if (!ev) {
        log_error("[CombinedNormalizedEventSource]: Component EventSource {} "
                  "couldn't "
                  "get the first event.",
                  src_it);
        comp_evs[src_it].first = kFinished;
      } else {
        comp_evs[src_it].first = kHasEvents;
        return ev;
      }
      break;
    }
    case kHasEvents: {
      ev = comp_evs[src_it].second->next();
      if (!ev) {
        comp_evs[src_it].first = kFinished;
      } else {
        return ev;
      }
      break;
    }
    default: {
    }
    }

    src_it = next_src();
  }

  return ev;
}

NormInfo CombinedNormalizedEventSource::norm_info(
    NuHepMC::CrossSection::Units::Unit const &units) {

  NormInfo combo_ni{0, 0, 0};

  for (auto &[status, evs] : comp_evs) {
    auto evs_ni = evs->norm_info(units);
    if (combo_ni.nevents == 0) {
      combo_ni = evs_ni;
    } else {
      combo_ni.fatx += evs_ni.fatx;
      combo_ni.sumweights += evs_ni.sumweights;
      combo_ni.nevents += evs_ni.nevents;
    }
  }

  if (combo_op == kAvg) {
    combo_ni.fatx /= double(comp_evs.size());
  }

  return combo_ni;
}

CombinedNormalizedEventSource::~CombinedNormalizedEventSource() {}

} // namespace nuis
