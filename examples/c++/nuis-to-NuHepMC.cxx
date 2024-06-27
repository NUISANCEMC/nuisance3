#include "nuis/eventinput/EventSourceFactory.h"

#include "NuHepMC/HepMC3Features.hxx"

#include "NuHepMC/make_writer.hxx"

#include "nuis/log.txx"

#include <iostream>
#include <memory>

int main(int argc, char const *argv[]) {
  
  if(argc < 3){
    std::cout << "Expected at least 2 arguments." << std::endl;
    return 1;
  }

  nuis::EventSourceFactory fact;
  auto [gri, evs] = fact.make(YAML::load_file(argv[1]));

  auto wrtr = NuHepMC::Writer::make_writer(argv[2], gri);

  size_t i = 0;
  for(auto const & [evt,cvw] : evs){
    wrtr->write_event(*evt);
    if(i && !(i%10000)){
      std::cout << "processed " << i << " events to NuHepMC" << std::endl;
    }
  }
  wrtr->close();

}