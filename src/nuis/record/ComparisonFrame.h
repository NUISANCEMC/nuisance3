#include "Comparison.h"
#include "HistFrame.h"
#include <string>

namespace nuis {
// Actually I think we do need inheritance here
// Then we can just do comparison["data"].content
struct ComparisonFrame : public HistFrame {

    std::string normalisation_type;
    std::string likelihood_type;
    bool by_bin_width;

    Eigen::ArrayXd correlation;

    ComparisonFrame::ComparisonFrame(Bins::BinOp bindef)
    : binning(bindef), column_info{{"data", ""}, {"mc", ""}} {
        Reset();
    };
};
}

