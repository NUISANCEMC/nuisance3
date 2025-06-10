# Event Frames

NUISANCE provides two DataFrame types for event-level data. Each provides a columnar table of projected event properties, where each included event corresponds to a single row in the table.

* [`nuis::EventFrame`](#nuiseventframe): A simple table stored in an [`Eigen::ArrayXXd`](https://eigen.tuxfamily.org/dox/group__TutorialArrayClass.html). This is useful for simple in-memory analysis where intermediate storage of the data is not required. Because it is stored in an Eigen Array, all columns are of double type. On the python side, the data can be directly interacted with as a two-dimensional [numpy ndarray](https://numpy.org/doc/stable/reference/generated/numpy.ndarray.html) without copying the underlying data.
* [`arrow::RecordBatch`](#arrowrecordbatch): [Apache Arrow](https://arrow.apache.org) is a standard in-memory, columnar analysis format that supports separately typed columns, batched I/O, and efficient compute processing. It also has first-class support for direct conversion to [Pandas DataFrames](https://pandas.pydata.org/docs/reference/api/pandas.DataFrame.html) for python-based analyses.

Both can be used to produce [tidy 'data'](http://vita.had.co.nz/papers/tidy-data.pdf) with the help of the [`nuis::EventFrameGen`](#creating-dataframes-with-nuiseventframegen) class.

This document describes the EventFrame types, facilities for producing them from an input event vector, and how to write/read `arrow::RecordBatch`s from files. For documentation on how they might be used in an analysis, see [HistFrames](../histframe/README.md).

## `nuis::EventFrame`

The `nuis::EventFrame` is designed to be used as a simple DataFrame-like object, consuming a vector of event data and executing the selection and projection loop, while carefully accounting for any important normalization information. 

The EventFrame `struct` is shown below:

```c++
struct EventFrame {
  std::vector<std::string> column_names;
  Eigen::ArrayXXd table;
  NormInfo norm_info;

  using column_t = uint16_t;
  constexpr static column_t const npos = std::numeric_limits<column_t>::max();

  column_t find_column_index(std::string const &name) const;

  Eigen::ArrayXdRef col(std::string const &cn);
  std::vector<Eigen::ArrayXdRef> cols(std::vector<std::string> const &cns);
};
```

The use of `Eigen::ArrayXdRef` allows individual columns to be fetched by name and manipulated in place. The `NormInfo` struct is defined [here](../eventinput/NormalizedEventSource.h) and contains information required to correctly normalize predictions made from data held within the `EventFrame`.

## Creating DataFrames with `nuis::EventFrameGen`

`EventFrame`s are generated with the `nuis::EventFrameGen` helper class, which works with `nuis::NormalizedEventSource` instances as below:

```c++
  EventSourceFactory fact;
  auto [gri, evs] = fact.Make("input.hepmc3");
  auto frame = EventFrameGen(evs).all();
  std::cout << frame << std::endl;
```

This may take a while to run depending on how many events are in, `input.hepmc3`. It will then print something like:

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
 |   10 |   1 |
 |   11 |   1 |
 |   12 |   1 |
 |   13 |   1 |
 |   14 |   1 |
 |   15 |   1 |
 |   16 |   1 |
 |   17 |   1 |
 |   18 |   1 |
 |   19 |   1 |
 |   20 |   1 |
 |  ... | ... |
 --------------
```

### Limiting The Number of Processed Events

We can limit the total number of events to process with the `limit` function:

```c++
  EventSourceFactory fact;
  auto [gri, evs] = fact.Make("input.hepmc3");
  auto frame = EventFrameGen(evs).limit(10).all();
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

#### Batched Processing

The most memory-efficient way to interact with `EventFrame`s is requesting and processing table rows in batches, rather than producing the whole table in memory in one go. Instead of requesting the entire EventFrame with `EventFrameGen::all`, we can request batches until no more events are available like:

```c++
auto fg = EventFrameGen(evs);
auto batch = fg.first();
while(batch){ // check if the batch has nay more rows
  for(auto row : batch.table.rowwise()){
    //do something with each row
  }
  batch = fg.next(); // get the next batch
}
```

The number of rows in each batch can be set in the `EventFrameGen` constructor like:

```c++
size_t batch_size = 250000;
auto fg = EventFrameGen(evs, batch_size);
```

### Adding Columns

By default a frame contains two columns, the first containing the `HepMC3::GenEvent::event_number` and the second containing the central value weight calculated by the `nuis::NormalizedEventSource`. We can add more columns with projection callables:

```c++

double enu(HepMC3::GenEvent const &ev){
  return ps::sel::Beam(ev)->momentum().e();
}

//..

  EventSourceFactory fact;
  auto [gri, evs] = fact.Make("input.hepmc3");
  auto frame = EventFrameGen(evs).limit(10).add_column("enu",enu).first();
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
  auto beamp = ps::sel::Beam(ev);
  return {beamp->momentum().e(), beamp->pid()};
}

//..

  auto frame = EventFrameGen(evs)
                 .limit(10)
                 .add_columns({"enu", "nupid"}, enu_nupid)
                 .first();
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

### Missing Entries

Missing datum should be signalled with `nuis::kMissingDatum<double>`, e.g.

```c++
double hmprotmom(HepMC3::GenEvent const &ev) {
  auto hmprot = ps::sel::OutPartHM(ev, 2212);
  return hmprot ? hmprot->momentum().p3mod() : nuis::kMissingDatum<double>;
}
```

### Filtering the Processed Events

Preselections can also be made with callables. If the callable returns a falsey result, the event will not produce a row not be inserted into the table. 

*N.B.* As the normalization information is handled by the `nuis::NormalizedEventSource`, the normalization information is correctly tallyed even if filtered events do not produce rows, we can check this with an example below:

```c++
struct single_procid_selector {
  int proc_id;
  single_procid_selector(int pid) : proc_id(pid) {}
  bool operator()(HepMC3::GenEvent const &ev) {
    return (NuHepMC::ER3::ReadProcessID(ev) == proc_id);
  }
};

//..

  auto frame = FrameGen(evs)
                 .limit(50)
                 .add_column("procid",NuHepMC::ER3::ReadProcessID)
                 .add_columns({"enu", "nupid"}, enu_nupid)
                 .filter(single_procid_selector(500))
                 .first();
  std::cout << frame << std::endl;
  std::cout << "NEvents Read: " <<  frame.norm_info.nevents << std::endl;
  std::cout << "NRows selected: " <<  frame.data.rows() << std::endl;
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

We have upped the event rate limit so that we still get a few events after the selection. The `procid` column shows the Process ID, which we also filter on using a functor to select events with `ProcessID == 500`. We can see that all 100 events read were included in the `norm_info`, which is important for correctly normalizing any predictions.

You can also add cross section reweights to a Frame using a wrapping lambda, as below:


```c++
  nuis::WeightCalcFactory wfact;
  auto wgt = wfact.Make(evs);
  wgt->SetParameters({
        {"ZExpA1CCQE", +2},
    });

  auto frame = FrameGen(evs)
          .limit(200)
          .add_column("procid", NuHepMC::ER3::ReadProcessID)
          .add_columns({"enu", "nupid"}, enu_nupid)
          .add_column("Zexp1Wght",
                      [=](auto const &ev) {
                        return wgt->CalcWeight(ev); 
                      })
          .filter(single_procid_selector(200))
          .first();
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

### A Warning for Weighters

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
          .limit(200)
          .add_column("procid", NuHepMC::ER3::ReadProcessID)
          .add_columns({"enu", "nupid"}, enu_nupid)
          .add_column({"Zexp1Wght_up","Zexp1Wght_down"},
                     [=](auto const &ev) {
  return {wgt1->CalcWeight(ev), wgt2->CalcWeight(ev)}; })
          .filter(single_procid_selector(200))
          .first();
  std::cout << frame << std::endl;
```

It is possible that the call `wgt2->SetParameters` overwrites the global memory used to store the results of calling
`wgt2->SetParameters`. Some may do what you expect and some may rip your arm off. And thats why... *we always sanity check 
our reweighting results!*

### Pretty Printing Options

By default, the pretty printer will only print the first 20 rows of the data and will signal that there are more rows in the data by printing a row of ellipses. To print more rows you can manually use the `nuis::FramePrinter` wrapper class to select the number of rows to print

```c++
  auto frame = FrameGen(evs).limit(1000).first();
  std::cout << FramePrinter(frame,30) << std::endl;
```

might print:

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
 |   10 |   1 |
 |   11 |   1 |
 |   12 |   1 |
 |   13 |   1 |
 |   14 |   1 |
 |   15 |   1 |
 |   16 |   1 |
 |   17 |   1 |
 |   18 |   1 |
 |   19 |   1 |
 |   20 |   1 |
 |   21 |   1 |
 |   22 |   1 |
 |   23 |   1 |
 |   24 |   1 |
 |   25 |   1 |
 |   26 |   1 |
 |   27 |   1 |
 |   28 |   1 |
 |   29 |   1 |
 |   30 |   1 |
 |  ... | ... |
 --------------
```

If you don't want to use the `nuis::Frame` pretty printer, and would rather just use the `Eigen` ostream overload, it can be disabled like:

```c++
  std::cout << FramePrinter(frame, 10, false) << std::endl;
```

resulting in:

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

You could also have just printed the `table` member directly:

```c++
  std::cout << frame.table.topRows(10) << std::endl;
```

## `arrow::RecordBatch`

If you intend to use `RecordBatch`es in your workflow, we strongly recommend reading the Arrow C++ documentation first: [Getting Started](https://arrow.apache.org/docs/cpp/getting_started.html) with Arrow. 

If Apache Arrow can be found on your system when the build is configured, NUISANCE's Arrow features will be enabled. `RecordBatches` are collections of columnar arrays of uniform length. Collections of `RecordBatches` with the same schema are called `arrow::Table`s. `RecordBatches`  can be produced by `EventFrameGen` by calling `EventFrameGen::firstArrow` and `EventFrameGen::nextArrow` rather than their `EventFrame` producing counterparts, `EventFrameGen::first` and `EventFrameGen::next`. These functions return `std::shared_ptr<arrow::RecordBatch>` instances. There is no `EventFrameGen::all` equivalent `EventFrameGen::allArrow` call, as working with batches of rows is the preferred way to process EventFrames. The number of rows in a RecordBatch is set by the second argument to `EventFrameGen` as for `EventFrames`. The maximum `arrow::Array` length, and thus `RecordBatch` size is 2,147,483,647 rows, so for most workflows you can practically work with a single in-memory `RecordBatch` by setting a very large `batch_size`, but we recommend against it.

```c++
size_t batch_size = 250000;
auto fg = EventFrameGen(evs, batch_size);
auto batch = fg.firstArrow();
while(batch){
  //do something with each batch
  batch = fg.nextArrow(); // get the next batch
}
```

### Typed Columns

The main difference between an `nuis::EventFrame` and `arrow::RecordBatch`, is that different columns in the `RecordBatch` can have different types, whereas all columns in an `EventFrame` are doubles. Arrow itself offers significant flexibility for column types, including nested types. However, we expect that the vast majority of our users will only need columns of numeric types and so to keep complexity minimal, EventFrameGen only supports simple numeric types. Typed columns can be declared like

```c++
double enu(HepMC3::GenEvent const &ev) {
  auto beamp = ps::sel::Beam(ev);
  return beamp->momentum().e();
}

int nupid(HepMC3::GenEvent const &ev) {
  auto beamp = ps::sel::Beam(ev);
  return beamp->pid();
}

//...

auto fg = EventFrameGen(evs);

fg.add_typed_column<double>("enu", enu);
fg.add_typed_column<int>("nupid", nupid);
```

The `EventFrameGen::add_column` and `EventFrameGen::add_columns` functions that we used when producing `EventFrame`s are actually just convenience functions calling `EventFrameGen::add_typed_column<double>` and `EventFrameGen::add_typed_columns<double>` respectively. `EventFrameGen` supports columns of the following types:

```c++
bool
int
uint
int16_t
uint16_t
float
double
```

Because C++ is strongly typed, when processing rows in a `RecordBatch`, you will need to know the type of a column and cast it appropriately. This should be done at the batch level so that typed field access does not require a cast. NUISANCE provides a helper function, `nuis::get_col_as<T>`, for casting the column accessor to the correct type, as shown in the complete example below:


```c++
double enu(HepMC3::GenEvent const &ev) {
  auto beamp = ps::sel::Beam(ev);
  return beamp->momentum().e();
}

int nupid(HepMC3::GenEvent const &ev) {
  auto beamp = ps::sel::Beam(ev);
  return beamp->pid();
}

//...

auto fg = EventFrameGen(evs);

fg.add_typed_column<double>("enu", enu);
fg.add_typed_column<int>("nupid", nupid);

auto batch = fg.firstArrow();
while(batch){
  //do the type casting here, at the batch level
  auto enu_col = nuis::get_col_as<double>(rbatch, "enu");
  auto nupid_col = nuis::get_col_as<int>(rbatch, "nupid");

  for(size_t row_it = 0; row_it < batch->num_rows(); ++row_it){
    if(nupid_col->Value(row_it) != 14){ // no row-level casting required
      continue;
    }
    myhist.fill(enu_col->Value(row_it));
  }

  batch = fg.nextArrow(); // get the next batch
}
```

### I/O with Arrow IPC

Once you have an `arrow::RecordBatch`, there are many options for onwards processing. In this section we give examples of how to write and read instances to files.

Arrow C++ makes heavy use of return codes and provides utility macros for error checking and
early return. An example of writing an `arrow::Table` to file is given below.

```c++
arrow::Status RunMain(int, char const *argv[]) {

  //...

  auto rbatch = frame.firstArrow();

  std::shared_ptr<arrow::io::FileOutputStream> outfile;
  ARROW_ASSIGN_OR_RAISE(outfile,
                        arrow::io::FileOutputStream::Open("test_out.arrow"));
  ARROW_ASSIGN_OR_RAISE(
      std::shared_ptr<arrow::ipc::RecordBatchWriter> ipc_writer,
      arrow::ipc::MakeFileWriter(outfile, rbatch->schema()));

  ARROW_RETURN_NOT_OK(ipc_writer->WriteRecordBatch(*rbatch));

  while (rbatch = frame.nextArrow()) {
    ARROW_RETURN_NOT_OK(ipc_writer->WriteRecordBatch(*rbatch));
  }

  ARROW_RETURN_NOT_OK(ipc_writer->Close());

  return arrow::Status::OK();
}

int main(int argc, char const *argv[]) {
  arrow::Status st = RunMain(argc, argv);
  if (!st.ok()) {
    std::cerr << st << std::endl;
    return 1;
  }
  return 0;
}
```