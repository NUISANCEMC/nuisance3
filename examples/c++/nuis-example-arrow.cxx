#include "nuis/eventinput/EventSourceFactory.h"

#include "nuis/eventframe/EventFrameGen.h"
#include "nuis/eventframe/column_types.h"
#include "nuis/eventframe/missing_datum.h"

#include "nuis/histframe/HistFrame.h"
#include "nuis/histframe/utility.h"

#include "NuHepMC/EventUtils.hxx"
#include "NuHepMC/ReaderUtils.hxx"

#include "ProSelecta/env/env.h"

#include "nuis/log.txx"

#include "arrow/api.h"
#include "arrow/io/api.h"
#include "arrow/ipc/api.h"

#include <iostream>
#include <memory>

std::vector<std::string> int_cols_names = {"process.id", "PDG.nu", "PDG.lep",
                                           "Target.A", "Target.Z"};
std::vector<int> int_cols(HepMC3::GenEvent const &ev) {

  std::vector<int> resp(int_cols_names.size(), nuis::kMissingDatum<int>);

  resp[0] = NuHepMC::ER3::ReadProcessID(ev);

  auto pin = ps::sel::BeamAny(ev, ps::pdg::groups::kNeutralLeptons);

  if (!pin) {
    return resp;
  }

  resp[1] = pin->pid();

  int nupid = pin->pid();
  int ccpid = nupid > 0 ? (nupid - 1) : (nupid + 1);

  auto pout = ps::sel::OutPartFirstAny(ev, {ccpid, nupid});

  if (!pout) {
    return resp;
  }

  resp[2] = pin->pid();

  auto tgt = NuHepMC::Event::GetTargetParticle(ev);

  if (!tgt) {
    return resp;
  }

  //   //±10LZZZAAAI
  resp[3] = (tgt->pid() / 10) % 1000;
  resp[4] = (tgt->pid() / 10000) % 1000;

  return resp;
}

std::vector<std::string> double_cols_names = {
    "true.nu.E",         "true.fslep.E",      "true.fslep.cos_theta",
    "true.event.lep.Q2", "true.event.lep.q0", "true.event.lep.q3",
};
std::vector<double> double_cols(HepMC3::GenEvent const &ev) {

  std::vector<double> resp(double_cols_names.size(),
                           nuis::kMissingDatum<double>);

  auto pin = ps::sel::BeamAny(ev, ps::pdg::groups::kNeutralLeptons);

  if (!pin) {
    return resp;
  }

  resp[0] = pin->momentum().e();

  int nupid = pin->pid();
  int ccpid = nupid > 0 ? (nupid - 1) : (nupid + 1);
  auto pout = ps::sel::OutPartFirstAny(ev, {ccpid, nupid});

  if (!pout) {
    return resp;
  }

  resp[1] = pout->momentum().e();
  resp[2] = std::cos(pout->momentum().theta());
  resp[3] = std::cos(pout->momentum().theta());

  resp[4] = ps::proj::parts::q0(pin, pout);
  resp[5] = ps::proj::parts::q3(pin, pout);
  resp[6] = ps::proj::parts::Q2(pin, pout);

  return resp;

  // double Emiss;
  // double pmiss;
  // double Enu_QE;
  // double Enu_true;
  // double Q2_QE;
  // double W_nuc_rest;
  // double W;
  // double x;
  // double y;
  // double Erecoil_minerva;
  // double Erecoil_charged;
  // double EavAlt;
  // double dalphat;
  // double dpt;
  // double dphit;
  // double pnreco_C;
  // double CosThetaAdler;
  // double PhiAdler;
}

void ProcessRBatch(std::shared_ptr<arrow::RecordBatch> &rbatch) {

  auto hf = nuis::HistFrame(nuis::Binning::lin_space(0, 2E3, 10));
  auto hf2 =
      nuis::HistFrame(nuis::Binning::lin_spaceND({{0, 2E3, 10}, {0, 2E3, 10}}));

  nuis::fill_from_RecordBatch(hf, rbatch, "true.event.lep.q0");
  nuis::fill_from_RecordBatch(
      hf, rbatch,
      std::vector<std::string>{"true.event.lep.q0", "true.event.lep.q3"});

  std::cout << hf << std::endl;
  std::cout << hf2 << std::endl;
}

arrow::Status RunMain(int, char const *argv[]) {

  nuis::EventSourceFactory fact;
  auto [gri, evs] = fact.make(argv[1]);

  if (!evs) {
    nuis::log_critical("Failed to find EventSource for input file {}", argv[1]);
    return arrow::Status::UnknownError(
        "Failed to find EventSource for input file {}", argv[1]);
  }

  auto frame = nuis::EventFrameGen(evs, 250000);

  frame.add_typed_column<bool>(
      "hasscclep", [](HepMC3::GenEvent const &ev) -> bool {
        auto pin = ps::sel::BeamAny(ev, ps::pdg::groups::kNeutralLeptons);
        if (!pin) {
          return false;
        }
        int nupid = pin->pid();
        int ccpid = nupid > 0 ? (nupid - 1) : (nupid + 1);
        auto pout = ps::sel::OutPartFirstAny(ev, {ccpid, nupid});

        if (!pout) {
          return false;
        }
        return true;
      });
  frame.add_typed_columns<int>(int_cols_names, int_cols);
  frame.add_typed_columns<double>(double_cols_names, double_cols);

  frame.limit(1E6);

  auto rbatch = frame.firstArrow();

  std::shared_ptr<arrow::io::FileOutputStream> outfile;
  ARROW_ASSIGN_OR_RAISE(outfile,
                        arrow::io::FileOutputStream::Open("test_out.arrow"));
  ARROW_ASSIGN_OR_RAISE(
      std::shared_ptr<arrow::ipc::RecordBatchWriter> ipc_writer,
      arrow::ipc::MakeFileWriter(outfile, rbatch->schema()));

  ARROW_RETURN_NOT_OK(ipc_writer->WriteRecordBatch(*rbatch));

  ProcessRBatch(rbatch);

  while (rbatch = frame.nextArrow()) {
    ARROW_RETURN_NOT_OK(ipc_writer->WriteRecordBatch(*rbatch));
  }

  ARROW_RETURN_NOT_OK(ipc_writer->Close());

  return arrow::Status::OK();
}

int main(int argc, char const *argv[]) {
  arrow::Status st = RunMain(argc, argv);
  if (!st.ok()) {
    std::cerr << st << std::endl;
    return 1;
  }
  return 0;
}