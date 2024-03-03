#pragma once

#include "nuis/histframe/Binning.h"
#include "nuis/histframe/HistFrame.h"

#include <string>

namespace nuis {
// Actually I think we do need inheritance here
// Then we can just do comparison["data"].content
struct ComparisonFrame {

    std::string normalisation_type;
    bool by_bin_width;

    Eigen::ArrayXd correlation;

    using column_t = uint32_t;

    HistFrame mc;
    HistFrame data;

    // Here its a bit unclear how to make the 0 column data...
    ComparisonFrame(Bins::BinOp bindef) {
        mc = HistFrame(bindef, "mc");
        data = HistFrame(bindef, "data");
    }

    // I don't really like this. Need to redict most fill commands to mc.
    column_t add_column(std::string const &name,
        std::string const &label = "") {
        return mc.add_column(name, label);
    }

    column_t find_column_index(std::string const &name) const {
        return mc.find_column_index(name);
    }

    Eigen::ArrayXd get_content(column_t col = 0,
                                bool divide_by_bin_sizes = false) const {
        return mc.get_content(col, divide_by_bin_sizes);
    }

    Eigen::ArrayXd get_error(column_t col = 0,
                            bool divide_by_bin_sizes = false) const {
        return mc.get_error(col, divide_by_bin_sizes);
    }

    HistColumn_View operator[](std::string const &name) const {
        if (name == "data") return data[name];
        return mc[name];
    }

    // Here DATA will still need its own assigned ID :(
    HistColumn_View operator[](column_t const& colid) const {
        return mc[colid];
    }

    Bins::BinId find_bin(std::vector<double> const &projections) const {
        return mc.find_bin(projections);
    }

    void fill(std::vector<double> const &projections, double weight,
                column_t col = 0) {
        mc.fill(projections, weight, col);
    }

    void fill_with_selection(int sel_int,
        std::vector<double> const &projections,
        double weight, column_t col = 0) {
        mc.fill_with_selection(sel_int, projections, weight, col);
    }

    void reset(){
        mc.reset();
    }
};
}

