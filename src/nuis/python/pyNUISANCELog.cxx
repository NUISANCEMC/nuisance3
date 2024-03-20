#include "nuis/log.txx"

#include "nuis/python/pyNUISANCE.h"

#include <string>

namespace py = pybind11;
using namespace nuis;

std::shared_ptr<spdlog::logger> get_ensure_logger(std::string const &name) {
  auto logger = spdlog::get(name);
  if (!logger) {
    logger = spdlog::stdout_color_mt(name);
    if (name == "default") {
      logger->set_pattern("[%l]: %v");
    } else {
      logger->set_pattern("[%n:%l]: %v");
    }
    logger->set_level(spdlog::level::warn);
    // std::cerr << "[pyLog]: Instantiated new logger: " << name << "("
    //           << logger.get() << ")" << std::endl;
  }
  // else {
  //   std::cerr << "[pyLog]: Fetched existing logger: " << name << "("
  //             << logger.get() << ")" << std::endl;
  // }
  return logger;
}

void pyNUISANCELogInit(py::module &m) {

  py::module logmod = m.def_submodule("log", "Logging functions");

  py::enum_<log_level>(logmod, "level")
      .value("trace", log_level::trace)
      .value("debug", log_level::debug)
      .value("info", log_level::info)
      .value("warn", log_level::warn)
      .value("error", log_level::error)
      .value("critical", log_level::critical)
      .value("off", log_level::off);

  logmod.def("trace", [](std::string const &s) { log_trace(s); })
      .def("debug", [](std::string const &s) { log_debug(s); })
      .def("info", [](std::string const &s) { log_info(s); })
      .def("warn", [](std::string const &s) { log_warn(s); })
      .def("error", [](std::string const &s) { log_error(s); })
      .def("critical", [](std::string const &s) { log_critical(s); })
      .def("set_level", [](log_level ll) { set_log_level(ll); })
      .def("get_level", []() { return get_log_level(); })
      .def("trace",
           [](std::string const &logger, std::string const &s) {
             get_ensure_logger(logger)->trace(s);
           })
      .def("debug",
           [](std::string const &logger, std::string const &s) {
             get_ensure_logger(logger)->debug(s);
           })
      .def("info",
           [](std::string const &logger, std::string const &s) {
             get_ensure_logger(logger)->info(s);
           })
      .def("warn",
           [](std::string const &logger, std::string const &s) {
             get_ensure_logger(logger)->warn(s);
           })
      .def("error",
           [](std::string const &logger, std::string const &s) {
             get_ensure_logger(logger)->error(s);
           })
      .def("critical",
           [](std::string const &logger, std::string const &s) {
             get_ensure_logger(logger)->critical(s);
           })
      .def("set_level",
           [](std::string const &logger, log_level ll) {
             switch (ll) {
             case log_level::trace: {
               get_ensure_logger(logger)->set_level(spdlog::level::trace);
               break;
             }
             case log_level::debug: {
               get_ensure_logger(logger)->set_level(spdlog::level::debug);
               break;
             }
             case log_level::info: {
               get_ensure_logger(logger)->set_level(spdlog::level::info);
               break;
             }
             case log_level::warn: {
               get_ensure_logger(logger)->set_level(spdlog::level::warn);
               break;
             }
             case log_level::error: {
               get_ensure_logger(logger)->set_level(spdlog::level::err);
               break;
             }
             case log_level::critical: {
               get_ensure_logger(logger)->set_level(spdlog::level::critical);
               break;
             }
             case log_level::off: {
               get_ensure_logger(logger)->set_level(spdlog::level::off);
               break;
             }
             }
           })
      .def("get_level",
           [](std::string const &logger) {
             switch (get_ensure_logger(logger)->level()) {
             case spdlog::level::trace: {
               return log_level::trace;
             }
             case spdlog::level::debug: {
               return log_level::debug;
             }
             case spdlog::level::info: {
               return log_level::info;
             }
             case spdlog::level::warn: {
               return log_level::warn;
             }
             case spdlog::level::err: {
               return log_level::error;
             }
             case spdlog::level::critical: {
               return log_level::critical;
             }
             default:
             case spdlog::level::off: {
               return log_level::off;
             }
             }
           })
      .def("get_macro_level", []() { return get_macro_log_level(); });
}
