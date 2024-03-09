# NUISANCEv3

Get in touch on [nuisance-xsec.slack.com](https://nuisance-xsec.slack.com).

## Getting NUISANCEv3

### Build From Source

Some dependencies (alma9):

```bash
dnf install -y yaml-cpp-devel boost-devel fmt-devel spdlog-devel eigen3-devel python3-devel
```

Build like:

```bash
cd /path/to/repo
mkdir build; cd build
cmake ../ -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  [-DNUISANCE_ENABLE_TESTS=OFF] \
  [-DNUISANCE_DEBUG_LEVEL=warn]
make install -j $(nproc)
```

Source the environment

```bash
source Linux/setup.nuis-eventinput.sh
printenv | grep -i nuisance
```

You can use the `picker24/nuisdev:alma9` container as an 'at-the-HEAD' development environment where NUISANCEv3 dependencies come pre-installed.

### Running in a Container

?? Put tutorial container here

## Getting Started with NUISANCEv3

Choose your development language: [python](#python) or [CPP](#cpp).

### python

For python bindings quick-start examples and documentation see [here](src/nuis/python/README.md).

### CPP

User documentation for the C++ API is broken up by module. See short summaries and links to detailed documentation below.

#### EventInput

* [Main Documentation](src/nuis/eventinput/README.md)
* [Generator Plugin Documentation](src/nuis/eventinput/plugins/README.md)

NUISANCEv3 provides ergonomic features for looping on events read from a variety of sources. Internally all events are `HepMC3::GenEvent`s but there are a number of generator-specific plugins included that can convert from other event formats on the fly.

#### Dataframes

* [`nuis::Frame`](src/nuis/frame/README.md)
* [`nuis::HistFrame`](src/nuis/histframe/README.md)


NUISANCEv3 provides two types of 'dateframe'. A `nuis::Frame` is intended to be used with event-level data where each row corresponds to a single event. A `nuis::HistFrame` is a minimal histogram-like frame where each row corresponds to a bin with N-dimensional extent.

## Contributing to NUISANCEv3

1. Read the [Developer README.md](README_ForDevelopers.md)
1. Discuss on [Slack](https://nuisance-xsec.slack.com)
1. Issue pull requests
1. Be a NUISANCE