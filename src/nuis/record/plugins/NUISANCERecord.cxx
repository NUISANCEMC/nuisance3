#include "nuis/record/plugins/IRecordPlugin.h"

#include "boost/dll/alias.hpp"

#include "nuis/log.txx"

#include "yaml-cpp/yaml.h"

#include <filesystem>

#include 

// THIS PLUGIN IS JUST A PLACEHOLDER

namespace nuis {

class NUISANCERecord : public IRecordPlugin {
 public:

    explicit NUISANCERecord(YAML::Node const &cfg) {
        std::cout << "LOADING NUISANCE " << std::endl;
        node = cfg;

        // Build the measurement class
        // Build the template at the start
        auto meas1d = dynamic_cast<Measurement1D*>(measurement);
        if (meas1d){
            meas1d->fDataHist;
        } 

        auto meas2d = dynamic_cast<Measurement2D*>(measurement);
        if (meas2d){
            meas2d->fDataHist;
        } 

        // Make an InputHandler
        inputhandler = NuHepMC3InputHandler();
    }

    TablePtr table(std::string name) {
        std::cout << "Getting NUISANCE " << name << std::endl;

        auto tab = Table();

        // Replace with Datasete
        ComparisonFrame hist(Bins::lin_space(100, 0, 10, "q_0 [GeV]"));
        tab.blueprint = std::make_shared<ComparisonFrame>(hist);

        tab.blueprint["data"].content  = 0.0;
        tab.blueprint["data"].variance = 0.0;

        tab.blueprint["mc"].content  = 0.0;
        tab.blueprint["mc"].variance = 0.0;

        tab.clear   = this->clear;
        tab.project = this->project;
        tab.weight  = this->weight;
        tab.select  = this->select;
        tab.finalize   = this->finalize;
        tab.likeihood  = this->likeihood;

        return std::make_shared<Table>(tab);
    }

    bool good() const { return true; }

    static IRecordPluginPtr Make(YAML::Node const &cfg) {
        return std::make_shared<NUISANCERecord>(cfg);
    }

    virtual ~NUISANCERecord() {}

    void clear(){
        cfr.reset();
    }

    std::vector<double> project(){

        (*inputhandler->HepMC3Event) = ev;
        inputhandler->CalcNUISANCEKinematics();

        // convert_event
        nuisance_measurement->FillEventVariables(inputhandler->fit_event);

    }

    int filter(){

        (*inputhandler->HepMC3Event) = ev;
        inputhandler->CalcNUISANCEKinematics();

        // convert_event
        return nuisance_measurement->isSignal(inputhandler->fit_event);
    }

    double weight(){
        return 1.0;
    }

    void finalize(){
        // Scale by norm info
    }

    MeasurementBase* nuisance_measurement;
};

BOOST_DLL_ALIAS(nuis::NUISANCERecord::Make,
                Make);

} // namespace nuis
