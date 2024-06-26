#pragma once

#include "boost/metaparse/string.hpp"

#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>

namespace spdlog {
class logger;
}

namespace nuis {

inline void StopTalking();
inline void StartTalking();

enum class log_level { trace, debug, info, warn, error, critical, off };

class log_level_scopeguard_impl {
  std::shared_ptr<spdlog::logger> logger;
  log_level old_level;

public:
  inline log_level_scopeguard_impl(std::shared_ptr<spdlog::logger>,
                                   log_level new_level);
  inline ~log_level_scopeguard_impl();
};

class stop_talking_scopeguard_impl {
  bool did_stop_talking;

public:
  inline stop_talking_scopeguard_impl(log_level ll, log_level limit);
  inline ~stop_talking_scopeguard_impl();
};

template <typename TN> class nuis_named_log_impl {

  static std::shared_ptr<spdlog::logger> logger;
  static void ensure_logger();

public:
  template <typename... Args> static void log_trace(Args &&...args);
  template <typename... Args> static void log_debug(Args &&...args);
  template <typename... Args> static void log_info(Args &&...args);
  template <typename... Args> static void log_warn(Args &&...args);
  template <typename... Args> static void log_error(Args &&...args);
  template <typename... Args> static void log_critical(Args &&...args);
  static void set_log_level(log_level);
  static log_level get_log_level();
  static log_level_scopeguard_impl log_level_scopeguard(log_level);
  static stop_talking_scopeguard_impl
  stop_talking_scopeguard(log_level ll = log_level::trace);
  static void envcfg();
};

template <typename... Args> void log_trace(Args &&...args);
template <typename... Args> void log_debug(Args &&...args);
template <typename... Args> void log_info(Args &&...args);
template <typename... Args> void log_warn(Args &&...args);
template <typename... Args> void log_error(Args &&...args);
template <typename... Args> void log_critical(Args &&...args);
inline void set_log_level(log_level);
inline log_level get_log_level();
inline log_level_scopeguard_impl log_level_scopeguard(log_level);
inline stop_talking_scopeguard_impl
stop_talking_scopeguard(log_level ll = log_level::trace);

template <typename T> std::string str_via_ss(T const &t) {
  std::stringstream ss;
  ss << t;
  return ss.str();
}

} // namespace nuis

#define nuis_named_log(TN) nuis::nuis_named_log_impl<BOOST_METAPARSE_STRING(TN)>
