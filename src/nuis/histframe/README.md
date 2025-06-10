# `HistFrame`

A `HistFrame` is a minimal histogram-like data frame. Rows correspond to bins. Columns can be added to group multiple histogram-like objects with identical binning together. See [here](../binning/README.md) for more rationale on why we decided to implement our own histogramming library.

Following [YODA2's rationale](https://scipost.org/10.21468/SciPostPhysCodeb.45), we draw a distinction between _live_ and _inert_ histogram types. A `HistFrame` is the _live_ type that can be _filled_ with new weighted entries and tracks the sum of fill weights and the sum of fill weights squared for each bin. A `HistFrame` can be _finalised_ to a `BinnedValues` instance, which is the corresponding _inert_ type, and tracks an abstract value and associated uncertainty for each bin. The operation of finalising a `HistFrame` to a `BinnedValues` often involves dividing the sum of each bin's fill weights by its hypervolume to calculate the corresponding _value_ as, in HEP, we often use histograms-like objects to estimate density functions, such as differential cross sections. The common interface to the two types is implemented by the `BinnedValuesBase` class.

## Quickstart

### C++

1) Make and fill a 1D `HistFrame` with a single entry, `1.234`, with weight `0.99`:

```c++
auto myhist = nuis::HistFrame(nuis::Binning::lin_space(-100, 100, 10));
myhist.fill(1.234, 0.99);
```

2) Make and fill a 1D `HistFrame` with two columns, one named `"CV"`, and one named `"Varied"`, filling each with a different weight:

```c++
auto myhist = nuis::HistFrame(nuis::Binning::lin_space(-100, 100, 10), "CV");
auto vcolid = myhist.add_column("Varied");
myhist.fill(1.234, 0.99); //Fills the first column by default
myhist.fill_column(1.234, 1.01, vcolid);
```

3) Make and fill a 2D `HistFrame`:

```c++
auto myhist = nuis::HistFrame(nuis::Binning::lin_spaceND({{-100, 100, 10},{0, 200, 50}}, {"x [xunits]", "y [yunits]"}), "CV", "dependent [units]");
myhist.fill({-1.234, 5.678}, 1.0);
```

4) Finalise a `HistFrame` once all fill operations have been completed

```c++
auto myhist = nuis::HistFrame(nuis::Binning::lin_space(-100, 100, 10));
for (auto const & [ev, cvw] : event_stream){
  myhist.fill(myprojection(ev), cvw);
}
auto mypdf = myhist.finalise(true);
```

5) Find the bin index corresponding to a certain value:

```c++
auto bini = myhist.binning.find_bin(5.0);

```

6) Retrieve the sum of weights and sum of weights squared in a given bin in a given column:

```c++
int coli = myhist.find_column_index("mycol");
auto sumweights = myhist[coli].sumweight[bini];
auto sumweightssquared = myhist[coli].variance[bini];
```

### Python

## Filling `HistFrame`s

All `HistFrame` fill functions require a weight to be passed explicitly. This may seem onerous, however, to produce correctly normalised cross-section predictions from generator event vectors without specialist knowledge, and event weight must *always* be used even if it is always `1` for a given specific generator.

`HistFrame`s can be filled explicitly entry-by-entry as demonstrated in [Quickstart](#quickstart). This approach is adequately performant in C++, but can be prohibitevly slow for event loops in Python. Furthermore, it is not ergonomic when working with columnar formats of pre-processed event data such as [`EventFrame`s](../eventframe/README.md) or `arrow::Table`s. NUISANCE provides functions for executing fill operations that process an entire dataframe in a single pass. A simple example is given below that fills a histogram from an event frame: a fill is performed for each row of the event frame if the value in the `"is_selected"` column of the event frame is non-zero, the bin index for the fill is determined from the value in the `"myprojection"` column, and each fill is weighted with the central value weight for that event:

```c++
nuis::fill(myhist, myeventframe, {"myprojection",},  nuis::fill_if("is_selected"));
```

### Choosing which column to fill

Columns from the input event frame can be used for a variety of operations beyond bin finding and row filtering. By default, the first column of the histogram is filled, an alternate column can be specified with `nuis::fill_column`, which takes either an integer or a string. If a string is passed, and no column with that name already exists on the histogram, it is created before filling. 

### Additional fill weights

Additional weights can be applied, which facilitates making predictions under systematic variations of the model, as the example below demonstrates:

```c++
nuis::fill(myhist, myeventframe, {"myprojection",},  nuis::fill_column(0), nuis::fill_if("is_selected"));
nuis::fill(myhist, myeventframe, {"myprojection",},  nuis::fill_column("varied"), nuis::fill_if("is_selected"), nuis::weight_by("variation_weight"));
```

Multiple column names can be passed to a single `weight_by` instance and the final event fill weight will be calculated from the multiplication of the entries from each referenced column. If instead, we want to create a new histogram column for each of many different sets of weights, naievly extending the above can get unwieldy, so a shortcut is provided:

```c++
nuis::fill(myhist, myeventframe, {"myprojection",},  nuis::fill_column(0), nuis::fill_if("is_selected"), nuis::weighted_column_map({"var1", "var2", "var3", ...}));
```

For each column name in the vector passed to `weighted_column_map`, a column on the histframe will be filled, weighted by the corresponding eventframe column. This procedure lets us translate event-level systematic variation weights to bin-level variation weights.

Sometimes it is preferable to not include the variation weights in an eventframe, especially if they need to be recalculated frequently, an `Eigen::ArrayXd` of weights can be passed directly, where the length of the array must match the number of rows in the eventframe.

```c++
Eigen::ArrayXd myweights = calc_weights();
nuis::fill(myhist, myeventframe, {"myprojection",}, nuis::weight_by_array(myweights));
```

If, for some reason, you need to disable the default behavior of weighting each fill by the event's CV weight, the `nuis::no_CV_weight()` operation can be used, however, this is an expert operation as there are few use cases for a prediction that doesn't include the CV weight.

### Filling sub-samples

Another common operation facilitated by the `nuis::fill` interface is keeping track of some sub-sampling of a total rate. This can be achieved with the `categorize_by` fill operation. The example below shows how to additionally fill an `HistFrame` column for each event based on the value in column, `"mytopology"`. If a vector of names is passed, then these will be used to name the `HistFrame` columns corresponding to integer values of the `"mytopology"` starting with `0`. The total prediction from all events in the event frame will be filled to column `0`, events with a value of `0` in the `"mytopology"` column will also be filled to column `"CC0Pi"` and so on. If the value in the `"mytopology"` column is larger than the array names, then only column `0` of the `HistFrame` will be filled for that event. If no category names are passed, then a column will be created for each unique value encounters, named, for example, `"mytopology:1337"`.

```c++
nuis::fill(myhist, myeventframe, {"myprojection",}, nuis::fill_column(0), nuis::categorize_by("mytopology", {"CC0Pi", "CC1Pi", "CCNPi", "NC"}));
```

A very common usage for this pattern is to breakdown predictions by the process identifier for the primary process in an event, a shortcut is provided for this case, as shown below:

```c++
nuis::fill(myhist, myeventframe, {"myprojection",}, nuis::split_by_ProcId());
```

The above will result in `HistFrame` columns named like `"process.id:400"`, _etc..._.

### Pre-binning

It can still be useful in relatively performance critical situations to use the same interface. The slowest part of a filling operation is usually the bin finding. For repeated filling of a HistFrame, the bin indexes for each event can be pre-calculated and passed to the `nuis::fill` operation like below:

```c++
nuis::fill(myhist, myeventframe, {}, nuis::prebinned("binid_column"));
```

or

```c++
auto allbins = nuis::find_bins(myhist.binning, myeventframe, {"myprojection",});
nuis::fill(myhist, myeventframe, {}, nuis::prebinned_array(allbins));
```

Note how here we can pass an empty array for the projection column names, as no bin finding is performed by `nuis::fill`.

## Finalizing `HistFrame`s

## Utility Functions

These utilities are provided as free functions that agnostically act on `const` instances of `HistFrames` and `BinnedValues` or combinations thereof to avoid cluttering the HistFrame interface. These functions are not designed for use in performance-critical code, usually the need to use such a function highlights poor design in the analysis: _e.g._ the use of higher dimensional intermediate histograms that are then projected and sliced before numeric analysis, instead perform analysis operations directly on the lower dimensional objects. However, for interactive exploratory analysis, or for producing figures of higher dimensional analysis objects they can be very useful. _n.b._ If you need to perform mathematical operations on the bin contents of a histogram in performance-critical code, `HistFrames::sumweights` and `HistFrames::variances` give direct access to the `Eigen::MatrixXXd` used as storage.


project
slice
tocount
tocountdensity
add/subtract/multiply/divide
finalize