#pragma once

#include "nuis/eventinput/IEventSource.h"

// this is required to enable gzip reading if we built in the support
#include "NuHepMC/HepMC3Features.hxx"

#include "HepMC3/ReaderFactory.h"

#include "spdlog/spdlog.h"

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/sum_kahan.hpp>

using namespace boost::accumulators;

namespace nuis {

class HepMC3EventSource : public IEventSource {

  std::string filepath;
  std::shared_ptr<HepMC3::Reader> reader;

  accumulator_set<double, stats<tag::sum_kahan>> sumw;

public:
  HepMC3EventSource(std::string const &fp) : filepath(fp){};

  std::optional<HepMC3::GenEvent> first() {
    //reset the weight counter, can't be bothered to copy out the type
    sumw = decltype(sumw)();

    // reopen the file from the start and get the next event
    reader = HepMC3::deduce_reader(filepath);
    if (!reader || reader->failed()) {
      spdlog::warn("Couldn't deduce reader for {} reader = {}, failed {}",
                   filepath, bool(reader), reader ? reader->failed() : false);
      return std::optional<HepMC3::GenEvent>();
    }
    return next();
  }

  std::optional<HepMC3::GenEvent> next() {
    if (reader->failed()) {
      return std::optional<HepMC3::GenEvent>();
    }

    HepMC3::GenEvent evt;
    reader->read_event(evt);
    if (reader->failed()) {
      return std::optional<HepMC3::GenEvent>();
    }

    sumw(evt.weights().front());

    return evt;
  }

  double sum_weights_so_far() { return sum_kahan(sumw); }

  std::shared_ptr<HepMC3::GenRunInfo> run_info() {
    return reader ? reader->run_info() : nullptr;
  }

  virtual ~HepMC3EventSource() {}
};

} // namespace nuis