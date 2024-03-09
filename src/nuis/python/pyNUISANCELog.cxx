#include "nuis/log.txx"

#include "pybind11/pybind11.h"

#include "spdlog/spdlog.h"

#include <string>

namespace py = pybind11;
using namespace nuis;

std::shared_ptr<spdlog::logger> get_ensure_logger(std::string const &name) {
  auto logger = spdlog::get(name);
  if (!logger) {
    logger = spdlog::stdout_color_mt(name);
    logger->set_pattern("[%n:%l]: %v");
    logger->set_level(spdlog::level::warn);
  }
  return logger;
}

void pyNUISANCELog(py::module &m) {

  py::module logmod = m.def_submodule("log", "Logging functions");

  py::enum_<log_level>(logmod, "level")
      .value("trace", log_level::trace)
      .value("debug", log_level::debug)
      .value("info", log_level::info)
      .value("warn", log_level::warn)
      .value("error", log_level::error)
      .value("critical", log_level::critical);

  logmod.def("trace", [](std::string const &s) { log_trace(s); })
      .def("debug", [](std::string const &s) { log_debug(s); })
      .def("info", [](std::string const &s) { log_info(s); })
      .def("warn", [](std::string const &s) { log_warn(s); })
      .def("error", [](std::string const &s) { log_error(s); })
      .def("critical", [](std::string const &s) { log_critical(s); })
      .def("set_level",
           [](log_level ll) {
             switch (ll) {
             case log_level::trace: {
               spdlog::set_level(spdlog::level::trace);
               break;
             }
             case log_level::debug: {
               spdlog::set_level(spdlog::level::debug);
               break;
             }
             case log_level::info: {
               spdlog::set_level(spdlog::level::info);
               break;
             }
             case log_level::warn: {
               spdlog::set_level(spdlog::level::warn);
               break;
             }
             case log_level::error: {
               spdlog::set_level(spdlog::level::err);
               break;
             }
             case log_level::critical: {
               spdlog::set_level(spdlog::level::critical);
               break;
             }
             }
           })
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
      .def("set_level", [](std::string const &logger, log_level ll) {
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
        }
      });
  //   set_log_level
  // log_trace
  // log_debug
  // log_info
  // log_warn
  // log_error
  // log_critical
  // set_log_level
}
