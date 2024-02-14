# NUISANCE C++ interface

## Measurement

The `Measurement` module provides methods for processing  inputs from external databases such as HEPData or EXFOR to make comparisons.

The base classes have been written around the minimal requirements for NUISANCE to generator a data-MC comparison, with the additional loader classes providing the functionality needed to interface with the databases themselves.

The loaders require a YAML::Node input to be configured.
```
auto config = YAML::Node();
config["type"] = "HEPData";
config["release"] = "[HEPID]";
config["table"] = "EventRate-Q2";

auto measurement = HEPDataLoader(config);
```



Every measurement loaded has the ability to generate a NUISANCE specific `Record` class, and functions to filter, weight, project, and correctly normalize events.

These functions rely on the ProSelecta package to provide a JIT compiled event processing interface when specific datasets are selected.

### Event Handling

```
auto data = measurement.CreateRecord("mc");

double w = fatx;
measurement.FinalizeRecord(h, w);
```


```
# Given some HEPMC3 event, ev
auto ev = source.first();

bool is_signal = measurement.FilterEvent(ev);

std::vector<double> xyz = measurement.ProjectEvent(ev);

double meas_weight = meaurement.WeightEvent(ev);
```



