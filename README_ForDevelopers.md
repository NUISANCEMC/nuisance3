# Developer Guidelines

Always a good place to start is the [CppCoreGuidelines](https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md#gsl-guidelines-support-library). We won't enforce them rigourously, but they are useful to internalise during your lunch breaks.

## Prefer `auto`

Use `auto` where you can. Type deduction is 👌.

## Naming Conventions

* `free_function_names`
* `ClassNames`
* `method_names`
* `MACRO_NAMES`

Rationale for method/free function naming style is to homogenise C++ and python interfaces.

## Headers

### include Guards

Prefer `#pragma once` over include guards, for reasons of pure laziness.

### include Statement Ordering

Bloomberg style guide: Broadly, order in ascending order of stability.

1. Corresponding header for current implementation, if applicable
1. NUISANCEv3 headers
1. 3rd party headers
1. Standard headers

Empty lines between include groups.

This guideline is pure style, we won't get too upset if you order them differently.

### Prefer quoted includes for non-system headers

### Don't use C headers directly, use C++ Versions

e.g. `#include <cmath>` instead of `#include <math.h>`

## Logging

Logging in NUISANCEv3 code should be done like:

```c++
#include "nuis/log.txx"

log_trace("my trace message");
log_debug("my debug message");
log_info("my info message");
log_warn("my warn message");
log_error("my error message");
log_critical("my critical message");
```

Behind the scenes these functions forward their arguments to `spdlog`, which means you get all the power of fmtlib for constructing your log messages. However, `spdlog` should not be used directly as we want to encapsulate this dependency away from user code. 

For hot code path logging that should be compiled out in release builds, see [Compile Time Log Levels](#compile-time-log-levels).

These functions can be called in any NUISANCEv3 implementation file that includes `"nuis/log.txx"`. Implementations should be kept out of header files so that we do not bleed our internal dependencies into user code.

### Named Loggers

We implement automatic logging to named loggers by providing the `nuis_named_log` macro, which should be used to subclass a class with static member functions templated over the logger name. We then take advantage of C++ name overload resolution ordering to have the static member functions be called, which forward to the named logger instances, rather than free functions with the same name, which forward to the default `spdlog` logger. Practically, this means that all logging should be done via the `log_<level>` functions above, but when logging from member functions of classes that subclass `nuis_named_log`, specific loggers will be transparently used.

An example of a new class which uses an automatically named trace:

```c++
//myclass.h
#include "nuis/log.h"

class myclass : public named_log_trace("mytrace") {
  void saysomething();
};

//myclass.cxx
#include "nuis/log.txx"

void myclass::saysomething(){ 
  log_info("some information"); //logs to the "mytrace" logger
}

void saysomethingfree(){
  log_info("some information"); //logs to the spdlog default logger
}
```

If myclass is not a subclass of `named_log_trace`, then the overload resolution will find the free functions and log to the default logger. There is a chance that this 'behind the scenes' choice of a logger may be confusing, but the worst that can happen is that your message accidently ends up with the default logger, rather than the named logger that you had hoped for.

If you would like to explicitly log to a named logger you can instead call:

```c++
named_log_trace("mytrace")::log_info("logging directly to mytrace");
```

from any execution block. If you are using a named trace a lot, it might improve compile times if you declare a using alias to the named trace type.

```c++
using mytrace = named_log_trace("mytrace");
mytrace::log_info("logging directly to mytrace");
```

New trace names will be automatically registered with spdlog on first use.

#### A Pitfall

If the logging call is fully qualified with the `nuis::` namespace, then this will stop the name overload resolution considering the static member functions. Write logging calls like without namespace qualification for the intended effect.

### Controlling the Log Level

#### With Environment Variables

```bash
NUIS_LOG_LEVEL_EventInput=trace
NUIS_LOG_LEVEL=trace
```

For example, running:

```bash
$ nuis-example-dumpevents test.hepmc3.gz
```

may not log any information about the attempts to parse the input file. If you would like the `"EventInput"` trace to be verbose, you might instead run:

```bash
$ NUIS_LOG_LEVEL_EventInput=trace nuis-example-dumpevents test.hepmc3.gz 
[EventInput:debug]: Found eventinput plugin: lib/plugins/nuisplugin-eventinput-GHEP3.so
[EventInput:debug]: Found eventinput plugin: lib/plugins/nuisplugin-eventinput-NuWroevent1.so
[EventInput:debug]: Found eventinput plugin: lib/plugins/nuisplugin-eventinput-neutvect.so
[EventInput:debug]: EventInputFactory: PathResolver::resolve filepath: test.hepmc3.gz, exists: true
[EventInput:debug]: Reading file test.hepmc3.gz with native HepMC3EventInput
```

#### With Function Calls

The trace log level is automatically set on first use from the environment, it can be changed at any time with:

```c++

void myfreefunc(){
  set_log_level(log_level::trace); // sets the default log level
}

myclass::mymethod(){
  set_log_level(log_level::debug); // sets the myclass named trace log level if myclass is a named_log_trace subclass, or the default log level if not
}
named_log_trace("mytrace")::set_log_level(log_level::info); // sets the mytrace named trace log level
using mytrace = named_log_trace("mytrace");
mytrace::set_log_level(log_level::warn); // sets the mytrace named trace log level
set_log_level(log_level::error);
set_log_level(log_level::critical);
```

### Compile Time Log Levels

Compile-time switchable logging macros can be used:

```c++
#include "nuis/log.txx"

NUIS_LOG_TRACE("Some trace message with param {}", 42);
NUIS_LOG_DEBUG("Some debug message"); // logs to the default or named trace subclass logger at the debug level
NUIS_LOGGER_INFO("some_trace","Some debug message"); // logs to the "some_trace" named trace logger at the info level
```

The compile-time log level is controlled with:

```bash
$ cmake -DNUISANCE_DEBUG_LEVEL=warn
```