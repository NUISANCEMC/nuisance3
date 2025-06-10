#include "nuis/eventinput/EventSourceFactory.h"

#include "nuis/record/RecordFactory.h"

#include "yaml-cpp/yaml.h"

#include "fmt/core.h"
#include "fmt/ranges.h"

using namespace nuis;

int main(int argc, char const *argv[]) {

  std::string recref = argv[1];

  std::string reftype = recref.find_first_of(':') == std::string::npos
                            ? "recordpath"
                            : "recordref";

  auto doc = YAML::Load(fmt::format(R"(
type: hepdata
{}: {}
)",
                                    reftype, recref));

  auto rf = RecordFactory();
  auto rec = rf.make(doc);

  if (argc <= 2) {
    std::cout << fmt::format("analyses: {}", rec->get_analyses()) << std::endl;
    return 0;
  }

  auto ana = rec->analysis(argv[2]);

  if (argc <= 3) {
    std::cout << ana->prediction_generation_hint() << std::endl;
    return 0;
  }

  std::vector<NormalizedEventSourcePtr> inputs;

  EventSourceFactory ef;

  for (int i = 3; i < argc; ++i) {
    inputs.push_back(ef.make(argv[i]).second);
  }

  if (inputs.size() == 1) {
    auto comp = ana->process(inputs.front());
    std::cout << "lhood: " << comp.likelihood() << std::endl;
  } else if (inputs.size()) {
    auto comp = ana->process(inputs);
    std::cout << "lhood: " << comp.likelihood() << std::endl;
  }
}
