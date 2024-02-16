# `nuis::Frame`

A `nuis::Frame` is an `Eigen::MatrixXd` holding selected and projected event data and a little extra metadata. It is designed to be used as a simple DataFrame-like object, consuming a vector of event data and executing the selection and projection loop, while carefully accounting for any important normalization information. The Frame `struct` is shown below:

```c++
struct Frame {
  std::vector<std::string> ColumnNames;
  Eigen::MatrixXd Table;
  NormInfo norm_info; 
};
```

where `NormInfo` is defined [here](../eventinput/INormalizedEventSource.h) and contains information required to correctly normalize

## Creating `Frame`s

`Frame`s are generated with the `nuis::FrameGen` interface which works with `nuis::INormalizedEventSource` instances as below:

```c++
  auto [gri, evs] = fact.Make("input.hepmc3");
  auto frame = FrameGen(evs).Evaluate();
  std::cout << frame.Table << std::endl;
```

This will write out a row for every event in the input file, `input.hepmc3`, which might be a lot. We can limit the number of events to process with the `Limit` function:

```c++
  auto [gri, evs] = fact.Make("input.hepmc3");
  auto frame = FrameGen(evs).Limit(10).Evaluate();
  std::cout << frame.Table << std::endl;
```

which will output:

```
0 1
1 1
2 1
3 1
4 1
5 1
6 1
7 1
8 1
9 1
```

By default a frame contains two columns, the first containing the `HepMC3::GenEvent::event_number` and the second containing the central value weight calculated by the `nuis::INormalizedEventSource`. We can add more columns with projection callables:

```c++

double enu(HepMC3::GenEvent const &ev){
  return NuHepMC::Event::GetBeamParticle(ev)->momentum().e();
}

//..

  auto [gri, evs] = fact.Make("input.hepmc3");
  auto frame = FrameGen(evs).Limit(10).AddColumn("enu",enu).Evaluate();
  std::cout << frame.Table << std::endl;
```

which will output:

```      
      0       1 12756.7
      1       1  1063.3
      2       1 27582.3
      3       1 2074.59
      4       1 2535.39
      5       1  2437.8
      6       1 2203.25
      7       1 14836.4
      8       1 2896.11
      9       1 3828.57
```

A single lambda can be used to return multiple projection columns, to reduce recalculating useful intermediate event properties, for example the neutrino energy and pid number.

```c++
std::vector<double> enu_nupid(HepMC3::GenEvent const &ev) {
  auto beamp = NuHepMC::Event::GetBeamParticle(ev);
  return {beamp->momentum().e(), beamp->pid()};
}

//..

  auto frame = FrameGen(evs)
                   .Limit(10)
                   .AddColumns({"enu", "nupid"}, enu_nupid)
                   .Evaluate();
  std::cout << frame.Table << std::endl;
```

which will output:

```
      0       1 12756.7      14
      1       1  1063.3      14
      2       1 27582.3      14
      3       1 2074.59      14
      4       1 2535.39      14
      5       1  2437.8      14
      6       1 2203.25      14
      7       1 14836.4      14
      8       1 2896.11      14
      9       1 3828.57      14
```

Preselections can also be made with callables. If an event fails the selection, a corresponding row will not be inserted into the table, but because the normalization information to accumulator when the event is retrieved from the `nuis::INormalizedEventSource`, the normalization information is correctly tallyed, we can check this with an example below:

```c++
struct single_procid_selector {
  int proc_id;
  single_procid_selector(int pid) : proc_id(pid) {}
  bool operator()(HepMC3::GenEvent const &ev){
    return (NuHepMC::ER3::ReadProcessID(ev) == proc_id);
  }
};

//..

  auto frame = FrameGen(evs)
                   .Limit(50)
                   .AddColumn("procid",NuHepMC::ER3::ReadProcessID)
                   .AddColumns({"enu", "nupid"}, enu_nupid)
                   .Filter(single_procid_selector(500))
                   .Evaluate();
  std::cout << frame.Table << std::endl;
  std::cout << "NEvents Read: " <<  frame.norm_info.nevents << std::endl;
  std::cout << "NRows selected: " <<  frame.Table.rows() << std::endl;
```

will output:

```
      0       1     500 12756.7      14
      3       1     500 2074.59      14
     10       1     500 2724.59      14
     14       1     500 3767.37      14
     27       1     500 1937.05      14
     31       1     500 2548.28      14
     33       1     500  2691.5      14
     36       1     500 1868.44      14
     37       1     500 3842.51      14
     38       1     500 3471.02      14
     40       1     500 3097.03      14
     48       1     500 9504.79      14
     49       1     500 3434.59      14
NEvents Read: 50
NRows selected: 13
```

We have upped the event rate limit so that we still get a few events after the selection. The first user column now shows the Process ID, which we also filter on using a simple functor to select events with `ProcessID == 500`. We can see that all 50 events read were included in the `norm_info` though, which is important for correctly normalizing any predictions.