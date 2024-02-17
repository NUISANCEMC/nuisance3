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
  EventSourceFactory fact;
  auto [gri, evs] = fact.Make("input.hepmc3");
  auto frame = FrameGen(evs).Evaluate();
  std::cout << frame << std::endl;
```

This will write out a row for every event in the input file, `input.hepmc3`, which might be a lot. We can limit the number of events to process with the `Limit` function:

```c++
  EventSourceFactory fact;
  auto [gri, evs] = fact.Make("input.hepmc3");
  auto frame = FrameGen(evs).Limit(10).Evaluate();
  std::cout << frame << std::endl;
```

which might output:

```
 --------------
 | evt# | cvw |
 --------------
 |    0 |   1 |
 |    1 |   1 |
 |    2 |   1 |
 |    3 |   1 |
 |    4 |   1 |
 |    5 |   1 |
 |    6 |   1 |
 |    7 |   1 |
 |    8 |   1 |
 |    9 |   1 |
 --------------
```

By default a frame contains two columns, the first containing the `HepMC3::GenEvent::event_number` and the second containing the central value weight calculated by the `nuis::INormalizedEventSource`. We can add more columns with projection callables:

```c++

double enu(HepMC3::GenEvent const &ev){
  return NuHepMC::Event::GetBeamParticle(ev)->momentum().e();
}

//..

  EventSourceFactory fact;
  auto [gri, evs] = fact.Make("input.hepmc3");
  auto frame = FrameGen(evs).Limit(10).AddColumn("enu",enu).Evaluate();
  std::cout << frame << std::endl;
```

which might output:

```      
 ----------------------
 | evt# | cvw |   enu |
 ----------------------
 |    0 |   1 | 10.23 |
 |    1 |   1 | 6.167 |
 |    2 |   1 | 2.748 |
 |    3 |   1 | 20.26 |
 |    4 |   1 | 20.37 |
 |    5 |   1 | 28.15 |
 |    6 |   1 | 25.98 |
 |    7 |   1 | 36.99 |
 |    8 |   1 | 1.619 |
 |    9 |   1 |  1.85 |
 ----------------------
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
  std::cout << frame << std::endl;
```

which might output:

```
 ------------------------------
 | evt# | cvw |   enu | nupid |
 ------------------------------
 |    0 |   1 | 10.23 |    14 |
 |    1 |   1 | 6.167 |    14 |
 |    2 |   1 | 2.748 |    14 |
 |    3 |   1 | 20.26 |    14 |
 |    4 |   1 | 20.37 |    14 |
 |    5 |   1 | 28.15 |    14 |
 |    6 |   1 | 25.98 |    14 |
 |    7 |   1 | 36.99 |    14 |
 |    8 |   1 | 1.619 |    14 |
 |    9 |   1 |  1.85 |    14 |
 ------------------------------
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
  std::cout << frame << std::endl;
  std::cout << "NEvents Read: " <<  frame.norm_info.nevents << std::endl;
  std::cout << "NRows selected: " <<  frame.Table.rows() << std::endl;
```

might output:

```
 ---------------------------------------
 | evt# | cvw | procid |   enu | nupid |
 ---------------------------------------
 |   12 |   1 |    500 |   3.1 |    14 |
 |   15 |   1 |    500 | 2.982 |    14 |
 |   28 |   1 |    500 |  2.81 |    14 |
 |   34 |   1 |    500 |   2.4 |    14 |
 |   35 |   1 |    500 | 1.561 |    14 |
 |   53 |   1 |    500 | 3.331 |    14 |
 |   64 |   1 |    500 | 3.183 |    14 |
 |   79 |   1 |    500 | 2.565 |    14 |
 |   80 |   1 |    500 | 5.951 |    14 |
 |   88 |   1 |    500 |  3.11 |    14 |
 |   95 |   1 |    500 | 5.544 |    14 |
 ---------------------------------------
NEvents Read:100
NRows selected:11
```

We have upped the event rate limit so that we still get a few events after the selection. The first user column now shows the Process ID, which we also filter on using a simple functor to select events with `ProcessID == 500`. We can see that all 100 events read were included in the `norm_info` though, which is important for correctly normalizing any predictions.

You can also add cross section reweights to a Frame using a wrapping lambda, as below:


```c++
  nuis::WeightCalcFactory wfact;
  auto wgt = wfact.Make(evs);
  wgt->SetParameters({
        {"ZExpA1CCQE", +2},
    });

  auto frame =
      FrameGen(evs)
          .Limit(200)
          .AddColumn("procid", NuHepMC::ER3::ReadProcessID)
          .AddColumns({"enu", "nupid"}, enu_nupid)
          .AddColumn("Zexp1Wght",
                     [=](auto const &ev) { return wgt->CalcWeight(ev); })
          .Filter(single_procid_selector(200))
          .Evaluate();
  std::cout << frame << std::endl;
```

which might output:

```
 ----------------------------------------------------
 | evt# | cvw | procid |    enu | nupid | Zexp1Wght |
 ----------------------------------------------------
 |    2 |   1 |    200 |  2.748 |    14 |    0.7618 |
 |   10 |   1 |    200 | 0.9284 |    14 |    0.8832 |
 |   19 |   1 |    200 |  3.491 |    14 |    0.5692 |
 |   43 |   1 |    200 |  20.88 |    14 |    0.8579 |
 |   49 |   1 |    200 |   1.73 |    14 |    0.6602 |
 |   60 |   1 |    200 |  2.572 |    14 |    0.8903 |
 |  115 |   1 |    200 |  2.132 |    14 |    0.7608 |
 |  159 |   1 |    200 |  1.233 |    14 |    0.3468 |
 |  162 |   1 |    200 |   4.35 |    14 |    0.5624 |
 |  182 |   1 |    200 |  8.652 |    14 |    0.8898 |
 |  187 |   1 |    200 |  2.553 |    14 |    0.7864 |
 |  189 |   1 |    200 |  3.915 |    14 |    0.6092 |
 ----------------------------------------------------
```

where the last column is the weight required to make a prediction with the `ZexpA1CCQE` parameter set to +2.

### Warning for ReWeighters

Some [weightcalc](../weightcalc) plugins wrap generator reweighting libraries that make extensive use of a global state and are not only thread unsafe, but the below may not do what you expect:

```c++
nuis::WeightCalcFactory wfact;
  auto wgt1 = wfact.Make(evs);
  wgt2->SetParameters({
        {"ZExpA1CCQE", +2},
    });
  auto wgt2 = wfact.Make(evs);
  wgt2->SetParameters({
        {"ZExpA1CCQE", -2},
    });

  auto frame =
      FrameGen(evs)
          .Limit(200)
          .AddColumn("procid", NuHepMC::ER3::ReadProcessID)
          .AddColumns({"enu", "nupid"}, enu_nupid)
          .AddColumn({"Zexp1Wght_up","Zexp1Wght_down"},
                     [=](auto const &ev) { return {wgt1->CalcWeight(ev), wgt2->CalcWeight(ev)}; })
          .Filter(single_procid_selector(200))
          .Evaluate();
  std::cout << frame << std::endl;
```

It is possible that the call `wgt2->SetParameters` overwrites the global memory used to store the results of calling
`wgt2->SetParameters`. Some may do what you expect and some may rip your arm off. And thats why... *we always sanity check 
our reweighting results!*