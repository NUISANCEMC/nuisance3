# NUISANCE Binning Library

An N dimensional binning is a function that maps R^N -> I. Often multi-dimensional.

## Quickstart

### C++

1) Make a 1D binning with 10 bins of equal width over the semi-open range [-100,100].

```c++
auto mybins = nuis::Binning::lin_space(-100, 100, 10);
```

**N.B.** the argument ordering, we decided to follow the `numpy.linspace(start, stop, num)` convention rather than the ROOT `TH1(num, start, stop)` convention for positional argument order.

2) Make a uniform 2D binning, include labels for each dimension.

```c++
auto mybins2d = nuis::Binning::lin_spaceND({{-100, 100, 10},{0, 200, 50}}, {"x [xunits]", "y [yunits]"});
```

3) Make a non-uniform, contiguous 1D binning from pre-defined bin edges.

```c++
std::vector<double> edges{-40,23,150,40000,1E16};
auto myotherbins = nuis::Binning::contiguous(edges, "weird_var [weirder units]");
```

The edges must be unique and monotonically increasing.

4) Combine any number of (possibly) multi-dimensional binnings to make a single higher dimensional binning

```c++
auto allthebins = nuis::Binning::product({mybins, mybins2d, myotherbins});
```

### Python

## Rationale

### Why another binning/histogramming library?

When designing NUISANCE3 we looked at existing histogramming solutions in an around HEP, specifically implementations offered by [ROOT](https://root.cern.ch/doc/master/classTH1.html), [Boost::Histogram](https://www.boost.org/doc/libs/1_86_0/libs/histogram/doc/html/index.html), and [YODA](https://yoda.hepforge.org), and none quite covered our problem space without custom extensions that we ultimately decided would eliminate the utility of using a pre-existing solution. We needed to be able to express arbitrary hyper-rectangular bins in an arbitrary number of dimensions. For arbitrary multi-dimensional histograms, YODA, and ROOT both enforce that the multi-dimensional bins are the product of independent 1D axes (TH2Poly from ROOT doesn't, but we didn't need fully polygonal binning and the TH2Poly interface appears less supported than the rest of the ROOT Hist library). There are already neutrino measurements in the wild that have multi-dimensional binning with non-uniform binning along multiple axes. We could have used any of these histogramming libraries as pure storage and defined our own binning functions mapping from the ND to 1D space, but that would have given us less flexibility to tailor our histogramming facilities to the NUISANCE solution space. So we decided to start from scratch. In the design, we have taken inspiration from all of the excellent existing solutions, as well as the HEPData YAML histogram format.

### Why no flow bins by default?

For differential cross section measurements under/overflow bins are problematic because they have no width. As noted by the YODA documentation, flow bins are useful for making projections from higher to lower dimensional space as entries that fall outside the bin range in some dimensions but not others should still contribute to a projection into the lower dimensional space. However, we made the decision that this was not worth the additional complexity of dealing with flow bins. If you really need flow bins, you can manually define them using `std::numeric_limits<double>::min()/max()`.

## In Detail

### Compatibility

#### ROOT

#### YODA

#### HEPData

## Binning (utility?) functions

can_project
slice
centers
centers1d
sizes
