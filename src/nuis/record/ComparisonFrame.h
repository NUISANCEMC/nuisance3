#pragma once

#include "nuis/histframe/Binning.h"
#include "nuis/histframe/HistFrame.h"

#include <string>

namespace nuis {
// Actually I think we do need inheritance here
// Then we can just do comparison["data"].content
struct ComparisonFrame : public HistFrame {

    std::string normalisation_type;
    bool by_bin_width;

    Eigen::ArrayXd correlation;

    ComparisonFrame(Bins::BinOp bindef,
        std::string const &def_col_name = "data",
        std::string const &def_col_label = ""){
        binning = bindef;
        column_info.push_back({def_col_name, def_col_label});
        column_info.push_back({"mc", ""});
        reset();
    }

};
}

