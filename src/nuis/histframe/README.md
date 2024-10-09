# `nuis::HistFrame`

A `HistFrame` is a minimal histogram-like data frame. Rows correspond to bins. Columns can be added to keep track of multiple histogram-like objects with identical binning concurrently.

## Filling `HistFrame`s

### From Events

### From EventFrames

```c++
fill_from_EventFrame
fill_from_EventFrame_if
fill_columns_from_EventFrame
fill_weighted_columns_from_EventFrame
fill_columns_from_EventFrame_if
fill_procid_columns_from_EventFrame
fill_procid_columns_from_EventFrame_if
```

#### From RecordBatches

Analogous functions exist for filling from arrow::RecordBatch event frames:

```c++
fill_from_RecordBatch
fill_from_RecordBatch_if
fill_columns_from_RecordBatch
fill_weighted_columns_from_RecordBatch
fill_columns_from_RecordBatch_if
fill_procid_columns_from_RecordBatch
fill_procid_columns_from_RecordBatch_if
```

fill(Arrow, fill_if({"column_name"}), weight_by({"column"}), fill_column(), pick_column_from(""))

## Finalizing `HistFrame`s

## functions
project
slice
tocount
tocountdensity
add/subtract/multiple/divide
finalize