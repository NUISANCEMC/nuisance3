from ._pyNUISANCE import EventFrameGen, EventFrame
from ._pyNUISANCE import HistFrame, BinnedValues, Binning

import matplotlib.pyplot as plt
import numpy as np
from copy import copy
from math import isfinite, nan

import bisect 

class EventFrameGen_mpl_helper():
    def __init__(self, fr):
        self.fr = fr

    def scatter(self, x, y, c=None, *args, **kwargs):
        df = self.fr.all()
        if c:
            c = df[c]
            kwargs["c"] = c
        obj = plt.scatter( df[x], df[y], *args, **kwargs )
        return obj
    
    def hist(self, x, y, weights="weight.cv", *args, **kwargs):
        df = self.fr.all()
        obj = plt.hist( df[x], self.df[weights], *args, **kwargs )
        return obj
    
    def hist2d(self, x, y, weights="weight.cv", *args, **kwargs):
        df = self.fr.all()
        obj = plt.hist2d( df[x], df[y], self.df[weights], *args, **kwargs )
        return obj

class EventFrame_mpl_helper():
    def __init__(self, df):
        self.df = df

    def scatter(self, x, y, *args, **kwargs):
        obj = plt.scatter( self.df[x], self.df[y], *args, **kwargs )
        return obj
    
    def hist(self, x, weights="weight.cv", *args, **kwargs):
        obj = plt.hist( self.df[x], weights=self.df[weights], *args, **kwargs )
        return obj
    
    def hist2d(self, x, y, weights="weight.cv", *args, **kwargs):
        print(self.df, x, y)
        obj = plt.hist2d( self.df[x], self.df[y], weights=self.df[weights], *args, **kwargs )
        return obj

class HistFrame_matplotlib_helper:
    def __init__(self, hf):
        self.hf = hf
        
    def x(self): return [(x[0].low+x[0].high)/2 for x in self.hf.binning.bins]
    def ex(self): return [(x[0].high-x[0].high)/2 for x in self.hf.binning.bins]
    def lx(self): return self.hf.binning.axis_labels[0]
    def bx(self): return ([(x[0].low) for x in self.hf.binning.bins]
                + [self.hf.binning.bins[-1][0].high])
    def by(self): return ([(x[1].low) for x in self.hf.binning.bins]
                + [self.hf.binning.bins[-1][1].high])
        
        
    def y(self): return [(x[1].low+x[1].high)/2 for x in self.hf.binning.bins]
    def ey(self): return [(x[1].high-x[1].high)/2 for x in self.hf.binning.bins]
    def ly(self): return self.hf.binning.axis_labels[1]
    
    def c(self, column=None): 
        if not column: column = self.hf.column_info[0].name
        if "count" in self.hf[column]: return self.hf[column]["count"]
        if "value" in self.hf[column]: return self.hf[column]["value"]

    def ec(self, column=None): 
        if not column: column = self.hf.column_info[0].name
        if "variance" in self.hf[column]: return np.sqrt(self.hf[column]["variance"])
        if "error" in self.hf[column]: return self.hf[column]["error"]

    def form_title(self, column, **kwargs):
        if "label" not in kwargs:
            kwargs["label"] = column
        return kwargs

    def get_1d_plotdim(self, axis):
        if axis == "x": 
            pdim = self.x()
            perr = self.ex()
            plab = self.lx()
        if axis == "y": 
            pdim = self.y()
            perr = self.ey()
            plab = self.ly()
        if axis == "z": 
            pdim = self.z()
            perr = self.ez()
            plab = self.lz()
        return pdim, perr, plab

    def get_1d_bins(self, axis):
        if axis == "x": 
            return ([(x[0].low) for x in self.hf.binning.bins]
                + [self.hf.binning.bins[-1][0].high])
            
    def errorbar(self, axis="x", column=None, plot_axis=None, *args, **kwargs):
        if not plot_axis: plot_axis = plt.gca()
        pdim, perr, plab = self.get_1d_plotdim(axis)
        obj = plot_axis.errorbar(pdim, self.c(column), xerr=perr, 
                            yerr=self.ec(), *args, **kwargs )
        plot_axis.set_xlabel(plab)
        return obj

    def plot(self, axis="x", column=None, fill=None, plot_axis=None, *args, **kwargs):
        if not plot_axis: plot_axis = plt.gca()
        pdim, perr, plab = self.get_1d_plotdim(axis)
        obj = plot_axis.plot(pdim, self.c(column), *args, **kwargs)
        if fill == "tozeroy":
            plot_axis.fill_between(pdim, np.zeros(len(pdim)), self.c(column))
        plot_axis.set_xlabel(plab)
        return obj
    
    def plot_all(self, axis="x", columns=None, labels=None, colors=None, plot_axis=None, *args, **kwargs):
        if not plot_axis: plot_axis = plt.gca()
        if not columns: columns = [x.name for x in self.hf.column_info]
        if not labels: labels = columns

        for i, (column, label) in enumerate(zip(columns,labels)):
            kwargs["label"] = label
            if not colors == None: kwargs["color"] = colors[i % len(colors)]
            self.plot(axis=axis, column=column, plot_axis=plot_axis, *args, **kwargs)
        return 

    def plot_sum(self, axis="x", columns=None, plot_axis=None, *args, **kwargs):
        if not plot_axis: plot_axis = plt.gca()
        if not columns: columns = [x.name for x in self.hf.column_info]

        pdim, perr, plab = self.get_1d_plotdim(axis)
        y = np.zeros(len(pdim))
        for column in columns:
          y = np.add(y, self.c(column))

        obj = plot_axis.plot(pdim, y, *args, **kwargs )
        plot_axis.set_xlabel(plab)
        return obj

    def fill(self, axis="x", column="mc", fill=None, plot_axis=None, *args, **kwargs):
        if not plot_axis: plot_axis = plt.gca()
        pdim, perr, plab = self.get_1d_plotdim(axis)
        obj = plot_axis.fill_between(pdim, np.zeros(len(pdim)), self.c(column), *args, **kwargs )
        return obj
        
    def fill_all(self, axis="x", columns=None, labels=None, colors=None, plot_axis=None, *args, **kwargs):
        if not plot_axis: plot_axis = plt.gca()
        if not columns: columns = [x.name for x in self.hf.column_info]
        if not labels: labels = columns

        for i, (column, label) in enumerate(zip(columns,labels)):
            kwargs["label"] = label
            if not colors == None: kwargs["color"] = colors[i % len(colors)]
            self.fill(axis=axis, column=column, plot_axis=plot_axis, *args, **kwargs)
        return 

    def scatter(self, axis="x", column=None, plot_axis=None, *args, **kwargs):
        if not plot_axis: plot_axis = plt.gca()
        pdim, perr, plab = self.get_1d_plotdim(axis)
        obj = plot_axis.scatter(pdim, self.c(column), *args, **kwargs )
        plot_axis.set_xlabel(plab)
        return obj

    def scatter_all(self, axis="x", columns=None, labels=None, plot_axis=None, colors=None, *args, **kwargs):
        if not plot_axis: plot_axis = plt.gca()
        if not columns: columns = [x.name for x in self.hf.column_info]
        if not labels: labels = columns

        for i, (column, label) in enumerate(zip(columns,labels)):
            kwargs["label"] = label
            if not colors == None: kwargs["color"] = colors[i % len(colors)]
            self.scatter(axis=axis, column=column, plot_axis=plot_axis, *args, **kwargs)
        return 
    
    def _hist_errorband(self, axis="x", column=None, plot_axis=None, color=None):
      if not plot_axis: plot_axis = plt.gca()

      yc = np.array([ c for c in self.c(column) ] + [ self.c(column)[-1] ])
      yerr = np.array([ c for c in self.ec(column) ] + [ self.ec(column)[-1] ])
    
      return plot_axis.fill_between(self.get_1d_bins(axis), yc+yerr, y2=yc-yerr, step="post", color=color)

    def hist(self, axis="x", column=None, plot_axis=None, *args, **kwargs):
        if not plot_axis: plot_axis = plt.gca()
        pdim, perr, plab = self.get_1d_plotdim(axis)

        errbar_obj = None
        if "errorband_color" in kwargs:
          errbar_obj = self._hist_errorband(axis, column, plot_axis, kwargs["errorband_color"])
          kwargs = copy(kwargs)
          kwargs.pop("errorband_color", None)
        
        obj = plot_axis.hist(pdim, weights=self.c(column), bins=self.get_1d_bins(axis), *args, **kwargs )
        plot_axis.set_xlabel(plab)
        
        if errbar_obj:
          return (obj, errbar_obj)
        else:
          return obj
    
    def hist_all(self, axis="x", columns=None, labels=None, colors=None, plot_axis=None, *args, **kwargs):
        if not plot_axis: plot_axis = plt.gca()
        if not columns: columns = [x.name for x in self.hf.column_info]
        if not labels: labels = columns

        for i, (column, label) in enumerate(zip(columns,labels)):
            kwargs["label"] = label
            if not colors == None: kwargs["color"] = colors[i % len(colors)]
            self.hist(axis=axis, column=column, plot_axis=plot_axis, *args, **kwargs)
        return 
 

    def hist2d(self, xaxis="x", yaxis="y", column=None, plot_axis=None, *args, **kwargs):
        if not plot_axis: plot_axis = plt.gca()
        pdim, perr, plab = self.get_1d_plotdim(xaxis)
        pdim2, perr2, plab2 = self.get_1d_plotdim(yaxis)
        obj = plot_axis.hist2d( pdim, pdim2, weights=self.c(column), bins=[np.unique(self.bx()), np.unique(self.by())], *args, **kwargs )
        plot_axis.set_xlabel(plab)
        plot_axis.set_ylabel(plab2)
        
        return obj
    
    def colormesh(self, xaxis="x", yaxis="y", column=None, plot_axis=None, *args, **kwargs):
        if not plot_axis: plot_axis = plt.gca()

        x_edges = []
        y_edges = []

        for i,bit in enumerate(Binning.get_sorted_bin_map(self.hf.binning.bins)):
          binext = self.hf.binning.bins[bit]

          if binext[0].low not in x_edges:
            bisect.insort(x_edges,binext[0].low)

          if binext[0].high not in x_edges:
            bisect.insort(x_edges,binext[0].high)

          if binext[1].low not in y_edges:
            bisect.insort(y_edges, binext[1].low)

          if binext[1].high not in y_edges:
            bisect.insort(y_edges, binext[1].high)


        X,Y = np.meshgrid(x_edges, y_edges)

        weights = self.c(column)

        # Placeholder for now will update to arb axis
        nbins = len(self.hf.binning.bins)
        X = np.zeros((2*nbins,2))
        Y = np.zeros((2*nbins,2))
        C = np.zeros(((2*nbins)-1,1))

        #    b) (X[i+1, j], Y[i+1, j])  d) (X[i+1, j+1], Y[i+1, j+1])
        #                       ●╶───╴●
        #                       │  i  │
        #                       ●╶───╴●
        #    a) (X[i, j], Y[i, j])      c) (X[i, j+1], Y[i, j+1])

        #draw along x and then y or you can get some weird artefacts
        for i,bit in enumerate(Binning.get_sorted_bin_map(self.hf.binning.bins)):
            binext = self.hf.binning.bins[bit]
            
            # a)
            X[2*i,0] = binext[0].low
            Y[2*i,0] = binext[1].low
            
            # b)
            X[2*i + 1,0] = binext[0].low
            Y[2*i + 1,0] = binext[1].high
            
            # c)
            X[2*i, 1] = binext[0].high
            Y[2*i, 1] = binext[1].low

            # d)
            X[2*i + 1, 1] = binext[0].high
            Y[2*i + 1, 1] = binext[1].high

            C[2*i,0] = weights[i]
            if (2*i + 1) != len(C):
                C[2*i + 1,0] = weights[i]
        
        return plot_axis.pcolormesh(X, Y, C)

def mpl_cern_template(page_dim=[3,3]):
    plt.minorticks_on()
    plt.rcParams.update({
        'figure.figsize': page_dim,
        'xtick.direction': 'in',
        'ytick.direction': 'in',
        'xtick.top': True,
        'ytick.right': True,
        'xtick.major.size': 6,
        'ytick.major.size': 6,
        'xtick.minor.size': 3,
        'ytick.minor.size': 3,
        'xtick.major.width': 1,
        'ytick.major.width': 1,
        'xtick.minor.width': 1,
        'ytick.minor.width': 1,
        'xtick.labelsize': 10,
        'ytick.labelsize': 10,
        'axes.labelsize': 12,
        'axes.titlesize': 14,
        'font.family': 'sans-serif',  # Set font family
        'font.serif': ['Times New Roman'],  # Set font to Times New Roman
        'axes.linewidth': 1.1,  # Increase axis linewidth for better visibility
        'axes.grid': False,  # Enable grid
        'axes.grid.which': 'major',  # Show both major and minor grid lines
        'axes.grid.axis': 'both',  # Show grid lines on both x and y axes
        'figure.dpi': 400,
        'legend.frameon': True,
        'legend.facecolor': 'none',  # Set legend background color
        'legend.edgecolor': 'none',  # Set legend border color to transparent
    })
    

#Dynamic Patching
def build_HistFrame_matplotlib_helper(self):
    return HistFrame_matplotlib_helper(self)

def build_EventFrameGen_mpl_helper(self):
    return EventFrameGen_mpl_helper(self)

def build_EventFrame_mpl_helper(self):
    return EventFrame_mpl_helper(self)

EventFrameGen.mpl = build_EventFrameGen_mpl_helper
HistFrame.mpl = build_HistFrame_matplotlib_helper
BinnedValues.mpl = build_HistFrame_matplotlib_helper
EventFrame.mpl = build_EventFrame_mpl_helper

HistFrame.show = plt.show

# naughty addition to mpl
cern = mpl_cern_template
