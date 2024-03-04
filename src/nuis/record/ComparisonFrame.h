#pragma once

#include "nuis/histframe/Binning.h"
#include "nuis/histframe/HistFrame.h"
#include <utility>
#include <string>


namespace nuis {
struct ComparisonFrame {

    std::string normalisation_type;
    bool by_bin_width;

    Eigen::ArrayXd correlation;

    using column_t = uint32_t;

    HistFrame mc;
    HistFrame data;

    ComparisonFrame(Bins::BinOp bindef) {
        mc = HistFrame(bindef, "mc");
        data = HistFrame(bindef, "data");
    }

    // Redirect most function calls down to mc as data is static
    template<typename... TS>
    auto add_column(TS&&... args)
        {return mc.add_column(std::forward<TS>(args)...);};

    template<typename... TS>
    auto find_column_index(TS&&... args)
        {return mc.find_column_index(std::forward<TS>(args)...);};

    template<typename... TS>
    Eigen::ArrayXd get_content(TS&&... args)
        {return mc.get_content(std::forward<TS>(args)...);};

    template<typename... TS>
    Eigen::ArrayXd get_error(TS&&... args)
        {return mc.get_error(std::forward<TS>(args)...);};

    template<typename... TS>
    void fill(TS&&... args)
        {return mc.fill(std::forward<TS>(args)...);};

    template<typename... TS>
    void fill_with_selection(TS&&... args)
        {return mc.fill_with_selection(std::forward<TS>(args)...);};

    template<typename... TS>
    auto find_bin(TS&&... args)
        {return mc.find_bin(std::forward<TS>(args)...);};

    template<typename... TS>
    void reset(TS&&... args)
        {return mc.reset(std::forward<TS>(args)...);};

    // Add some data functions
    template<typename... TS>
    Eigen::ArrayXd get_data_content(TS&&... args)
        {return data.get_content(std::forward<TS>(args)...);};

    template<typename... TS>
    Eigen::ArrayXd get_data_error(TS&&... args)
        {return data.get_error(std::forward<TS>(args)...);};

    HistColumn_View operator[](std::string const &name) const {
        if (name == "data") return data[name];
        return mc[name];
    }

    // Here DATA will still need its own assigned ID :(
    HistColumn_View operator[](column_t const& colid) const {
        return mc[colid];
    }

};
}

