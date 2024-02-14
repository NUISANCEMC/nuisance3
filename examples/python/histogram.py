import pyNUISANCE as pn

b = pn.hepdata.bin()
print(b.Summary())

b.extent_low  = [0.5,1.0]
b.extent_high = [1.0,2.0]

print(b.InRange( [0.5,1.0] ))

b.FillInRange([0.5,1.0], 1)
b.FillInRange([20.0,1.0], 1)

print(b.content, b.entries)


test = {
	"header": {
		"name": "MyVar",
		"title": "Test",
		"units": "testunits"
	},
	"values": [
		{"value": 5},
		{"value": 10}
	]
	}

hd = pn.hepdata.variables()

print(hd.summary())
#b.configure(test)

mysetup = { "measurement": "T2K_Analysis1", 
            "release": "../../data/T2K",
            "analysis": "../../data/T2K/analysis.cxx", 
            "table": "CrossSection-CosThetaMuPMu_AnalysisI", 
            "input": {"file": "../../../runneut/neutvect_nu.root", "weight": 0.23} }
loader = pn.hepdata.measurement(mysetup)
print(loader.measurement_name)
print(loader.independent_variables)

for v in loader.independent_variables: print(v.summary())

h = loader.createhistogram()
print(h)
print(h.summary())

events = pn.generator.reader( mysetup )



