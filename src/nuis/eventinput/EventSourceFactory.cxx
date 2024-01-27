#include "nuis/eventinput/HepMC3EventSource.h"

#include "boost/dll/import.hpp"
#include "boost/function.hpp"

#include "yaml-cpp/yaml.h"

namespace nuis {
class EventSourceFactory {

  typedef std::unique_ptr<IEventSource>(IEventSource_PluginFactory_t)(
      YAML::Node const &);
  std::unordered_map<std::string, boost::function<IEventSource_PluginFactory_t>>
      pluginfactories;

  boost::function<IEventSource_PluginFactory_t> &
  GetPluginFactory(std::string const &pname) {
    if (pluginfactories.count(pname)) {
      return pluginfactories[pname];
    }

    auto NUIS_PLUGINS = std::getenv("NUIS_PLUGINS");

    if (!NUIS_PLUGINS) {
      std::cout << "NUIS_PLUGINS not defined" << std::endl;
      abort();
    }

    boost::dll::fs::path shared_library_path(NUIS_PLUGINS);

    shared_library_path /= pname + "EventSource";

    pluginfactories[pname] =
        boost::dll::import_alias<IEventSource_PluginFactory_t>(
            shared_library_path, std::string("Make_") + pname + "EventSource",
            boost::dll::load_mode::append_decorations);

    return pluginfactories[pname];
  }

public:
  std::unique_ptr<IEventSource> Make(YAML::Node const &cfg) {
    auto type = cfg["type"].as<std::string>();

    if (type == "HepMC3") {
      return std::make_unique<HepMC3EventSource>(
          cfg["filepath"].as<std::string>());
    } else if (type == "neutvect") {
      return GetPluginFactory("neutvect")(cfg);
    }
    return nullptr;
  }
};
} // namespace nuis