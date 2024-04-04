#pragma once

#include "nuis/log.h"

#ifndef NUISANCE_LOG_ENABLED
#error                                                                         \
    "nuis/log.txx should not be included by user code. Are our headers leaking?"
#endif

#define SPDLOG_DISABLE_DEFAULT_LOGGER

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

#include <iostream>

namespace nuis {

inline log_level to_nuis_log_level(spdlog::level::level_enum sl) {
  switch (sl) {
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
}

inline spdlog::level::level_enum to_spdlog_level(log_level ll) {
  switch (ll) {
  case log_level::trace: {
    return spdlog::level::trace;
  }
  case log_level::debug: {
    return spdlog::level::debug;
  }
  case log_level::info: {
    return spdlog::level::info;
  }
  case log_level::warn: {
    return spdlog::level::warn;
  }
  case log_level::error: {
    return spdlog::level::err;
  }
  case log_level::critical: {
    return spdlog::level::critical;
  }
  default:
  case log_level::off: {
    return spdlog::level::off;
  }
  }
}

inline log_level_scopeguard_impl::log_level_scopeguard_impl(
    std::shared_ptr<spdlog::logger> lgr, log_level new_level)
    : logger(lgr) {
  old_level = to_nuis_log_level(logger->level());
  logger->set_level(to_spdlog_level(new_level));
}

inline log_level_scopeguard_impl::~log_level_scopeguard_impl() {
  logger->set_level(to_spdlog_level(old_level));
}

template <typename TN>
std::shared_ptr<spdlog::logger> nuis_named_log_impl<TN>::logger = nullptr;

template <typename TN> void nuis_named_log_impl<TN>::ensure_logger() {
  if (!logger) {
    logger =
        spdlog::get(boost::mpl::c_str<TN>::value); // check that it hasn't been
                                                   // registered already

    if (!logger) {
      logger = spdlog::stdout_color_mt(boost::mpl::c_str<TN>::value);
      // std::cout << "[log]: Instantiated new logger: "
      //           << boost::mpl::c_str<TN>::value << ", " << logger.get()
      //           << std::endl;
      if (std::string("default") == boost::mpl::c_str<TN>::value) {
        logger->set_pattern("[%l]: %v");
      } else {
        logger->set_pattern("[%n:%l]: %v");
      }
      logger->set_level(spdlog::level::warn);
      envcfg();
    }
    // else {
    // std::cout << "[log]: Fetched existing logger: "
    //           << boost::mpl::c_str<TN>::value << ", " << logger.get()
    //           << std::endl;
    // }
  }
}

template <typename TN>
template <typename... Args>
void nuis_named_log_impl<TN>::log_trace(Args &&...args) {
  ensure_logger();
  logger->trace(std::forward<Args>(args)...);
}
template <typename TN>
template <typename... Args>
void nuis_named_log_impl<TN>::log_debug(Args &&...args) {
  ensure_logger();
  logger->debug(std::forward<Args>(args)...);
}
template <typename TN>
template <typename... Args>
void nuis_named_log_impl<TN>::log_info(Args &&...args) {
  ensure_logger();
  logger->info(std::forward<Args>(args)...);
}
template <typename TN>
template <typename... Args>
void nuis_named_log_impl<TN>::log_warn(Args &&...args) {
  ensure_logger();
  logger->warn(std::forward<Args>(args)...);
}
template <typename TN>
template <typename... Args>
void nuis_named_log_impl<TN>::log_error(Args &&...args) {
  ensure_logger();
  logger->error(std::forward<Args>(args)...);
}
template <typename TN>
template <typename... Args>
void nuis_named_log_impl<TN>::log_critical(Args &&...args) {
  ensure_logger();
  logger->critical(std::forward<Args>(args)...);
}

template <typename TN>
void nuis_named_log_impl<TN>::set_log_level(log_level ll) {
  ensure_logger();
  logger->set_level(to_spdlog_level(ll));
}

template <typename TN> log_level nuis_named_log_impl<TN>::get_log_level() {
  ensure_logger();
  return to_nuis_log_level(logger->level());
}

template <typename TN> void nuis_named_log_impl<TN>::envcfg() {
  auto envname =
      (std::string(boost::mpl::c_str<TN>::value) == "default")
          ? "NUIS_LOG_LEVEL"
          : fmt::format("NUIS_LOG_LEVEL_{}", boost::mpl::c_str<TN>::value);
  auto envv = std::getenv(envname.c_str());
  if (envv) {
    std::string envs(envv);
    if (envs == "trace") {
      set_log_level(log_level::trace);
    } else if (envs == "debug") {
      set_log_level(log_level::debug);
    } else if (envs == "info") {
      set_log_level(log_level::info);
    } else if (envs == "warn") {
      set_log_level(log_level::warn);
    } else if (envs == "error") {
      set_log_level(log_level::error);
    } else if (envs == "critical") {
      set_log_level(log_level::critical);
    } else if (envs == "off") {
      set_log_level(log_level::off);
    } else {
      std::cerr << fmt::format("[WARNING]: Invalid log level set in \"{}\"",
                               envname)
                << std::endl;
    }
  }
}

template <typename TN>
log_level_scopeguard_impl
nuis_named_log_impl<TN>::log_level_scopeguard(log_level ll) {
  ensure_logger();
  return log_level_scopeguard_impl(logger, ll);
}

template <typename TN>
stop_talking_scopeguard_impl
nuis_named_log_impl<TN>::stop_talking_scopeguard(log_level ll) {
  ensure_logger();
  return stop_talking_scopeguard_impl(ll, get_log_level());
}

template <typename... Args> void log_trace(Args &&...args) {
  nuis_named_log("default")::log_trace(std::forward<Args>(args)...);
}
template <typename... Args> void log_debug(Args &&...args) {
  nuis_named_log("default")::log_debug(std::forward<Args>(args)...);
}
template <typename... Args> void log_info(Args &&...args) {
  nuis_named_log("default")::log_info(std::forward<Args>(args)...);
}
template <typename... Args> void log_warn(Args &&...args) {
  nuis_named_log("default")::log_warn(std::forward<Args>(args)...);
}
template <typename... Args> void log_error(Args &&...args) {
  nuis_named_log("default")::log_error(std::forward<Args>(args)...);
}
template <typename... Args> void log_critical(Args &&...args) {
  nuis_named_log("default")::log_critical(std::forward<Args>(args)...);
}
inline void set_log_level(log_level ll) {
  nuis_named_log("default")::set_log_level(ll);
}

inline log_level get_log_level() {
  return nuis_named_log("default")::get_log_level();
}

inline log_level_scopeguard_impl log_level_scopeguard(log_level ll) {
  return nuis_named_log("default")::log_level_scopeguard(ll);
}

inline stop_talking_scopeguard_impl::stop_talking_scopeguard_impl(
    log_level ll, log_level limit)
    : did_stop_talking{false} {
      std::cout << "ll: " << int(ll) << ", limit = " << int(limit) << std::endl;
  if (ll <= limit) {
    std::cout << " did scope guard " << std::endl;
    did_stop_talking = true;
    StopTalking();
  };
}
inline stop_talking_scopeguard_impl::~stop_talking_scopeguard_impl() {
  if (did_stop_talking) {
    StartTalking();
  std::cout << "stop scope guard" << std::endl;
  }
}

inline stop_talking_scopeguard_impl stop_talking_scopeguard(log_level ll) {
  return stop_talking_scopeguard_impl(ll, get_log_level());
}

} // namespace nuis

#define NUIS_LEVEL_TRACE 0
#define NUIS_LEVEL_DEBUG 1
#define NUIS_LEVEL_INFO 2
#define NUIS_LEVEL_WARN 3
#define NUIS_LEVEL_ERROR 4
#define NUIS_LEVEL_CRITICAL 5

namespace nuis {
inline auto get_macro_log_level() {
#if (NUIS_ACTIVE_LEVEL == NUIS_LEVEL_CRITICAL)
  return log_level::critical;
#endif
#if (NUIS_ACTIVE_LEVEL == NUIS_LEVEL_ERROR)
  return log_level::error;
#endif
#if (NUIS_ACTIVE_LEVEL == NUIS_LEVEL_WARN)
  return log_level::warn;
#endif
#if (NUIS_ACTIVE_LEVEL == NUIS_LEVEL_INFO)
  return log_level::info;
#endif
#if (NUIS_ACTIVE_LEVEL == NUIS_LEVEL_DEBUG)
  return log_level::debug;
#endif
#if (NUIS_ACTIVE_LEVEL == NUIS_LEVEL_TRACE)
  return log_level::trace;
#endif
  return log_level::off;
}

static std::streambuf
    *default_cout; //!< Where the STDOUT stream is currently directed
static std::streambuf
    *default_cerr; //!< Where the STDERR stream is currently directed
static std::ofstream
    redirect_stream; //!< Where should unwanted messages be thrown

inline void StopTalking() {
  std::cout.rdbuf(redirect_stream.rdbuf());
  std::cerr.rdbuf(redirect_stream.rdbuf());
  // shhnuisancepythiaitokay_();
  // fflush(stdout);
  // fflush(stderr);
  // dup2(silentfd, fileno(stdout));
  // dup2(silentfd, fileno(stderr));
}

inline void StartTalking() {
  std::cout.rdbuf(default_cout);
  std::cerr.rdbuf(default_cerr);
  // canihaznuisancepythia_();
  // fflush(stdout);
  // fflush(stderr);
  // dup2(savedstdoutfd, fileno(stdout));
  // dup2(savedstderrfd, fileno(stderr));
}

} // namespace nuis

#if (NUIS_ACTIVE_LEVEL <= NUIS_LEVEL_TRACE)
#define NUIS_LOG_TRACE(...) log_trace(__VA_ARGS__)
#else
#define NUIS_LOG_TRACE(...)
#endif

#if (NUIS_ACTIVE_LEVEL <= NUIS_LEVEL_DEBUG)
#define NUIS_LOG_DEBUG(...) log_debug(__VA_ARGS__)
#else
#define NUIS_LOG_DEBUG(...)
#endif

#if (NUIS_ACTIVE_LEVEL <= NUIS_LEVEL_INFO)
#define NUIS_LOG_INFO(...) log_info(__VA_ARGS__)
#else
#define NUIS_LOG_INFO(...)
#endif

#if (NUIS_ACTIVE_LEVEL <= NUIS_LEVEL_WARN)
#define NUIS_LOG_WARN(...) log_warn(__VA_ARGS__)
#else
#define NUIS_LOG_WARN(...)
#endif

#if (NUIS_ACTIVE_LEVEL <= NUIS_LEVEL_ERROR)
#define NUIS_LOG_ERROR(...) log_error(__VA_ARGS__)
#else
#define NUIS_LOG_ERROR(...)
#endif

#if (NUIS_ACTIVE_LEVEL <= NUIS_LEVEL_CRITICAL)
#define NUIS_LOG_CRITICAL(...) log_critical(__VA_ARGS__)
#else
#define NUIS_LOG_CRITICAL(...)
#endif

#if (NUIS_ACTIVE_LEVEL <= NUIS_LEVEL_TRACE)
#define NUIS_LOGGER_TRACE(logger, ...)                                         \
  nuis_named_log(logger)::log_trace(__VA_ARGS__)
#else
#define NUIS_LOGGER_TRACE(logger, ...)
#endif

#if (NUIS_ACTIVE_LEVEL <= NUIS_LEVEL_DEBUG)
#define NUIS_LOGGER_DEBUG(logger, ...)                                         \
  nuis_named_log(logger)::log_debug(__VA_ARGS__)
#else
#define NUIS_LOGGER_DEBUG(logger, ...)
#endif

#if (NUIS_ACTIVE_LEVEL <= NUIS_LEVEL_INFO)
#define NUIS_LOGGER_INFO(logger, ...)                                          \
  nuis_named_log(logger)::log_info(__VA_ARGS__)
#else
#define NUIS_LOGGER_INFO(logger, ...)
#endif

#if (NUIS_ACTIVE_LEVEL <= NUIS_LEVEL_WARN)
#define NUIS_LOGGER_WARN(logger, ...)                                          \
  nuis_named_log(logger)::log_warn(__VA_ARGS__)
#else
#define NUIS_LOGGER_WARN(logger, ...)
#endif

#if (NUIS_ACTIVE_LEVEL <= NUIS_LEVEL_ERROR)
#define NUIS_LOGGER_ERROR(logger, ...)                                         \
  nuis_named_log(logger)::log_error(__VA_ARGS__)
#else
#define NUIS_LOGGER_ERROR(logger, ...)
#endif

#if (NUIS_ACTIVE_LEVEL <= NUIS_LEVEL_CRITICAL)
#define NUIS_LOGGER_CRITICAL(logger, ...)                                      \
  nuis_named_log(logger)::log_critical(__VA_ARGS__)
#else
#define NUIS_LOGGER_CRITICAL(logger, ...)
#endif
