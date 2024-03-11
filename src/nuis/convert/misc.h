namespace plotly {
std::string to_1D_json(HistFrame const &hf);
}

namespace matplotlib {
std::map<std::string, Eigen::ArrayXXd>
to_pcolormesh_data(HistFrame const &hf, HistFrame::column_t colid = 0);
}