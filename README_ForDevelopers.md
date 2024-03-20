# Developer Guidelines

Always a good place to start is the [CppCoreGuidelines](https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md#gsl-guidelines-support-library). We won't enforce them rigourously, but they are useful to internalise during your lunch breaks.

* [Developer Tips](#nuisancev3-developer-tips)
  * [Exceptions](#exceptions)
  * [Logging](#logging)
* [Diagnostics](#diagnostics)
  * [Profiling](#profiling)
  * [Sanitizers](#sanitizers)
* [Style](#style)

## NUISANCEv3 Developer Tips

### Exceptions

Declare new exception types and throw them when exceptional things happen.

Only throw an exception if execution cannot reasonably continue. Being forced to otherwise make assumptions in possibly confusing ways and masking user errors counts as not being able to reasonably continue.

```c++
#include "nuis/except.h"

namespace nuis {
NEW_NUISANCE_EXCEPT(MyTypedException);
}

void myfunc(){
  if (something.isbad()) {
    throw MyTypedException()
        << "write an error message to be displayed by the runtime if the "
           "exception is not caught."; // don't use std::endl, it will cause a
                                       // compiler error
  }
}

```

### Logging

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

#### Named Loggers

We implement automatic logging to named loggers by providing the `nuis_named_log` macro, which defines a class template with static member logging functions templated over the logger name. We then take advantage of C++ name overload resolution ordering to have the static member functions be called, in `nuis_named_log` subclasses, which forward to the named logger instances, rather than free functions with the same name, which forward to the default `spdlog` logger. Practically, this means that all logging should be done via the `log_<level>` functions above, but when logging from member functions of classes that subclass `nuis_named_log`, specific loggers will be transparently used.

An example of a new class which uses an automatically named logger:

```c++
//myclass.h
#include "nuis/log.h"

class myclass : public named_log_trace("mylogger") {
  void saysomething();
};

//myclass.cxx
#include "nuis/log.txx"

void myclass::saysomething(){ 
  log_info("some information"); //logs to the "mylogger" logger
}

void saysomethingfree(){
  log_info("some information"); //logs to the spdlog default logger
}
```

If myclass is not a subclass of `named_log_trace`, then the overload resolution will find the free functions and log to the default logger. There is a chance that this 'behind the scenes' choice of a logger may be confusing, but the worst that can happen is that your message accidently ends up with the default logger, rather than the named logger that you had hoped for.

If you would like to explicitly log to a named logger you can instead call:

```c++
named_log_trace("mylogger")::log_info("logging directly to mylogger");
```

from any execution block. It can improve readability if you declare a `using` alias to the named logger type.

```c++
using mylogger = named_log_trace("mylogger");
mylogger::log_info("logging directly to mylogger");
```

New trace names will be automatically registered with spdlog on first use.

If you would like to use the same logger as a known class, you can call it as a static function on that class, like:

```c++
void saysomethingfree2(){
  myclass::log_info("some information"); //logs to the "mylogger" logger as myclass subclasses nuis_named_log("mylogger")
}
```

This will cause a compiler error if `myclass` does not publicly inherit from `named_log_trace`.

##### A Pitfall

If the logging call is fully qualified with the `nuis::` namespace, then this will stop the name overload resolution considering the static member functions. Write logging calls without namespace qualification for the intended effect. Use `using namespace nuis` if you have to (but not in a header file).

#### Controlling the Log Level

##### With Environment Variables

```bash
NUIS_LOG_LEVEL_EventInput=trace
NUIS_LOG_LEVEL=trace
```

For example, running:

```bash
$ nuis-example-dumpevents test.hepmc3.gz
```

may not log any information about the attempts to parse the input file. If you would like the `"EventInput"` logger to be verbose, you might instead run:

```bash
$ NUIS_LOG_LEVEL_EventInput=trace nuis-example-dumpevents test.hepmc3.gz 
[EventInput:debug]: Found eventinput plugin: lib/plugins/nuisplugin-eventinput-GHEP3.so
[EventInput:debug]: Found eventinput plugin: lib/plugins/nuisplugin-eventinput-NuWroevent1.so
[EventInput:debug]: Found eventinput plugin: lib/plugins/nuisplugin-eventinput-neutvect.so
[EventInput:debug]: EventInputFactory: PathResolver::resolve filepath: test.hepmc3.gz, exists: true
[EventInput:debug]: Reading file test.hepmc3.gz with native HepMC3EventInput
```

##### With Function Calls

The logger level is automatically set on first use from the environment, it can be changed at any time with:

```c++

void myfreefunc(){
  set_log_level(log_level::trace); // sets the default log level
}

myclass::mymethod(){
  set_log_level(log_level::debug); // sets the myclass named logger level if myclass is a named_log_trace subclass, or the default log level if not
}
named_log_trace("mylogger")::set_log_level(log_level::info); // sets the mylogger named logger level
using mylogger = named_log_trace("mylogger");
mylogger::set_log_level(log_level::warn); // sets the mylogger named logger level
set_log_level(log_level::error);
myclass::set_log_level(log_level::critical); // sets the myclass named logger level if myclass is a named_log_trace subclass, or causes a a compiler error if not
```

#### With Scope Guards

If you want to set a new logging level temporarily and have it set back after the current scope ends you can use a `log_level_scopeguard` like:

```c++
auto scopeguard = myclass::log_level_scopeguard(log_level::trace); //set myclass logger level to trace until scopeguard falls out of scope
```

#### Compile Time Log Levels

Compile-time switchable logging macros can be used:

```c++
#include "nuis/log.txx"

NUIS_LOG_TRACE("Some trace message with param {}", 42);
NUIS_LOG_DEBUG("Some debug message"); // uses the default logger or named logger subclass logger at the debug level
NUIS_LOGGER_INFO("some_logger","Some debug message"); // logs to the "some_logger" named loggerger at the info level
```

The level of logging calls that are compiled in is set by:

```bash
$ cmake -DNUISANCE_DEBUG_LEVEL=warn
```

The default for `Release` and `RelWithDebInfo` builds is `warn`, and for `Debug` the default is `trace`.

*N.B.* This level just determines what level of messages are compiled in/out of the binary. If thye are compiled in, then they will respect log levels for the relevant default or named logger.

As a result, you should instrument your code liberally with `NUIS_LOG_TRACE`/`NUIS_LOG_DEBUG` calls, as they will not spam stdout unless you lower the logger level, and they will not impact performance in Releasey builds.

Get the current compile-time level with like:

```c++
#include "nuis/log.txx"

nuis::get_macro_log_level();
```

## Diagnostics

### Profiling

Make sure you have gperftools set up, preferably via the system package manager: e.g. `dnf install -y gperftools pprof`.

Link NUISANCE to gperftools by configuring the build like:

```bash
cmake .. -DNUISANCE_ENABLE_GPERFTOOLS=ON
```

#### CPU Profiling

Run code like:

```bash
CPUPROFILE=frame.prof nuis-example-frame ../notebooks/dune_argon_sf_10mega.nuwro.pb.gz 
```

Make pdf of call graph like:

```bash
pprof --pdf Linux/bin/nuis-example-frame frame.prof > frame.prof.pdf
```

#### HEAP Profiling

#### Sanitizers

Make sure you have ASAN and UBSAN set up, preferably via the system package manager: e.g. `dnf install -y libasan libubsan`.

Link NUISANCE to the sanitizers by configuring the build like:

```bash
cmake .. -DNUISANCE_ENABLE_SANITIZERS=ON
```

Then run your code as normal, it will take somewhat longer to run (say 2-3x as long), but will be able to notify you of many possible problems in your code.

Interpreting the reports is the next challenge though...

## Style

### Prefer `auto`

Use `auto` where you can. Type deduction is 👌.

### Naming Conventions

* `free_function_names`
* `ClassNames`
* `method_names`
* `MACRO_NAMES`
* `ClassDefinition.h`
* `utilities.h`

Rationale for method/free function naming style is to homogenise C++ and python interfaces.

### Headers

#### include Guards

Prefer `#pragma once` over include guards, for reasons of pure laziness.

#### include Statement Ordering

Bloomberg style guide: Broadly, order in ascending order of stability.

1. Corresponding header for current implementation, if applicable
1. NUISANCEv3 headers
1. 3rd party headers
1. Standard headers

Empty lines between include groups.

This guideline is pure style, we won't get too upset if you order them differently.

#### Prefer quoted includes for non-system headers

#### Don't use C headers directly, use C++ Versions

e.g. `#include <cmath>` instead of `#include <math.h>`
