#include <Eigen/Dense>
#include <vector>

#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wsign-compare"

#include "nuis/measurement/Record.h"
#include "nuis/measurement/SimpleStatistics.h"

namespace nuis {
namespace measurement {

double CalculateRecordLikelihood(const nuis::measurement::Record& record) {
    // Returns record likelihood based on data mc conversion
    Eigen::VectorXd residuals(record.data_value.size());
    for (size_t i = 0; i < record.data_value.size(); i++) {
        residuals(i) =\
            (record.data_value[i] - record.mc_weights[i]);
        i++;
    }

    Eigen::MatrixXd covariance(record.data_value.size(),
        record.data_value.size());

    for (size_t i = 0; i < record.data_value.size(); i++) {
        for (size_t j = 0; j < record.data_value.size(); j++) {
            covariance(i, j) = record.data_covariance[i][j];
        }
    }

    Eigen::MatrixXd inverse_covariance = covariance.inverse();

    // Now run our chi2 calculation
    double chi2 = (residuals.transpose() * covariance.inverse() * residuals);

    return chi2;
}


// double CalculateDialLikelihood(const WeightManager& manager) {
//     // Returns record likelihood based on data mc conversion
//     Eigen::MatrixXd residuals(manager.dial_values.size());
//     for (size_t i = 0; i < manager.dial_values.size(); i++) {
//         residuals(i) =
//             (manager.dial_values[i] - manager.dial_nominals[i])/
//             (manager.dial_errors[i]);
//     }

//     Eigen::MatrixXd correlation(manager.dial_values.size(),
//         manager.dial_values.size());

//     for (size_t i = 0; i < manager.dial_values.size(); i++) {
//         correlation[i].push_back(std::vector<double>());
//         for (size_t j = 0; j < manager.dial_values.size(); j++) {
//             eigenMatrix(i, j) = manager.dial_correlations[i][j];
//         }
//     }

//     Eigen::MatrixXd inverse_correlation = correlation.inverse();

//     // Now run our chi2 calculation
//     double chi2 = 
//     (residualVector.transpose() * correlationMatrix.inverse() * residualVector);

//     return chi2;
// }


}  // namespace statistical
}  // namespace nuis 
