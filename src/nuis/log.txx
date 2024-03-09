#include "nuis/log.h"

#ifndef NUISANCE_LOG_ENABLED
#error                                                                         \
    "nuis/log.txx should not be included by user code. Are our headers leaking?"
#endif

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include <iostream>
    
namespace nuis {

template <typename TN>
std::shared_ptr<spdlog::logger> nuis_named_log_impl<TN>::logger = nullptr;

template <typename TN> void nuis_named_log_impl<TN>::ensure_logger() {
  if (!logger) {
    logger = spdlog::stdout_color_mt(boost::mpl::c_str<TN>::value);
    logger->set_pattern("[%n:%l]: %v");
    logger->set_level(spdlog::level::warn);
    envcfg();
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
  switch (ll) {
  case log_level::trace: {
    logger->set_level(spdlog::level::trace);
    break;
  }
  case log_level::debug: {
    logger->set_level(spdlog::level::debug);
    break;
  }
  case log_level::info: {
    logger->set_level(spdlog::level::info);
    break;
  }
  case log_level::warn: {
    logger->set_level(spdlog::level::warn);
    break;
  }
  case log_level::error: {
    logger->set_level(spdlog::level::err);
    break;
  }
  case log_level::critical: {
    logger->set_level(spdlog::level::critical);
    break;
  }
  }
}
template <typename TN> void nuis_named_log_impl<TN>::envcfg() {
  auto envname = fmt::format("NUIS_LOG_LEVEL_{}", boost::mpl::c_str<TN>::value);
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
    } else {
      std::cerr << fmt::format("[WARNING]: Invalid log level set in \"{}\"",
                               envname)
                << std::endl;
    }
  }
}

template <typename... Args> void log_trace(Args &&...args) {
  spdlog::trace(std::forward<Args>(args)...);
}
template <typename... Args> void log_debug(Args &&...args) {
  spdlog::debug(std::forward<Args>(args)...);
}
template <typename... Args> void log_info(Args &&...args) {
  spdlog::info(std::forward<Args>(args)...);
}
template <typename... Args> void log_warn(Args &&...args) {
  spdlog::warn(std::forward<Args>(args)...);
}
template <typename... Args> void log_error(Args &&...args) {
  spdlog::error(std::forward<Args>(args)...);
}
template <typename... Args> void log_critical(Args &&...args) {
  spdlog::critical(std::forward<Args>(args)...);
}
inline void set_log_level(log_level ll) {
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
}
} // namespace nuis

#define NUIS_LEVEL_TRACE 0
#define NUIS_LEVEL_DEBUG 1
#define NUIS_LEVEL_INFO 2
#define NUIS_LEVEL_WARN 3
#define NUIS_LEVEL_ERROR 4
#define NUIS_LEVEL_CRITICAL 5

#if (NUIS_ACTIVE_LEVEL >= NUIS_LEVEL_TRACE)
#define NUIS_LOG_TRACE(...) nuis::log_trace(__VA_ARGS__)
#else
#define NUIS_LOG_TRACE(...)
#endif

#if (NUIS_ACTIVE_LEVEL >= NUIS_LEVEL_DEBUG)
#define NUIS_LOG_DEBUG(...) nuis::log_debug(__VA_ARGS__)
#else
#define NUIS_LOG_DEBUG(...)
#endif

#if (NUIS_ACTIVE_LEVEL >= NUIS_LEVEL_INFO)
#define NUIS_LOG_INFO(...) nuis::log_info(__VA_ARGS__)
#else
#define NUIS_LOG_INFO(...)
#endif

#if (NUIS_ACTIVE_LEVEL >= NUIS_LEVEL_WARN)
#define NUIS_LOG_WARN(...) nuis::log_warn(__VA_ARGS__)
#else
#define NUIS_LOG_WARN(...)
#endif

#if (NUIS_ACTIVE_LEVEL >= NUIS_LEVEL_ERROR)
#define NUIS_LOG_ERROR(...) nuis::log_error(__VA_ARGS__)
#else
#define NUIS_LOG_ERROR(...)
#endif

#if (NUIS_ACTIVE_LEVEL >= NUIS_LEVEL_CRITICAL)
#define NUIS_LOG_CRITICAL(...) nuis::log_critical(__VA_ARGS__)
#else
#define NUIS_LOG_CRITICAL(...)
#endif

#if (NUIS_ACTIVE_LEVEL >= NUIS_LEVEL_TRACE)
#define NUIS_LOGGER_TRACE(logger, ...)                                         \
  nuis_named_log(logger)::log_trace(__VA_ARGS__)
#else
#define NUIS_LOGGER_TRACE(logger, ...)
#endif

#if (NUIS_ACTIVE_LEVEL >= NUIS_LEVEL_DEBUG)
#define NUIS_LOGGER_DEBUG(logger, ...)                                         \
  nuis_named_log(logger)::log_debug(__VA_ARGS__)
#else
#define NUIS_LOGGER_DEBUG(logger, ...)
#endif

#if (NUIS_ACTIVE_LEVEL >= NUIS_LEVEL_INFO)
#define NUIS_LOGGER_INFO(logger, ...)                                          \
  nuis_named_log(logger)::log_info(__VA_ARGS__)
#else
#define NUIS_LOGGER_INFO(logger, ...)
#endif

#if (NUIS_ACTIVE_LEVEL >= NUIS_LEVEL_WARN)
#define NUIS_LOGGER_WARN(logger, ...)                                          \
  nuis_named_log(logger)::log_warn(__VA_ARGS__)
#else
#define NUIS_LOGGER_WARN(logger, ...)
#endif

#if (NUIS_ACTIVE_LEVEL >= NUIS_LEVEL_ERROR)
#define NUIS_LOGGER_ERROR(logger, ...)                                         \
  nuis_named_log(logger)::log_error(__VA_ARGS__)
#else
#define NUIS_LOGGER_ERROR(logger, ...)
#endif

#if (NUIS_ACTIVE_LEVEL >= NUIS_LEVEL_CRITICAL)
#define NUIS_LOGGER_CRITICAL(logger, ...)                                      \
  nuis_named_log(logger)::log_critical(__VA_ARGS__)
#else
#define NUIS_LOGGER_CRITICAL(logger, ...)
#endif
