# Usage

## Fully Auto

```python
data_release = "hepdata/12345"
table_name = "cross_section"
comp = np.Comparison(data_release,table_name).process()

comp.plot()
```

## Steered generation, automatic file -> comparison

```python
data_release = "hepdata/12345"
table_name = "cross_section"
comp = np.Comparison(data_release,table_name)

#generate events
gen_help = np.GenerationHelper(comp)
os.system(gen_help.command())

evs = np.EventSource(gen_help.file_name())

comp.process(evs)

comp.plot()
```

## Existing file, steered table build

```python
data_release = "hepdata/12345"
table_name = "cross_section"
comp = np.Comparison(data_release,table_name)

evs = np.EventSource("myfile.hepmc")

efg = np.EventFrameGen(evs)

comp.add_columns(efg)

ef = efg.all()

comp.process(ef)
```