#pragma once

#include "TDirectory.h"
#include "TError.h"
#include "TFile.h"
#include "TTree.h"

#include "spdlog/spdlog.h"

#include <filesystem>
#include <fstream>

inline bool IsROOTFile(std::filesystem::path filepath) {

  if (!std::filesystem::exists(filepath)) {
    return false;
  }
  std::ifstream fin(filepath);
  char magicbytes[5];
  fin.read(magicbytes, 4);
  magicbytes[4] = '\0';
  if (std::string(magicbytes) != "root") {
    return false;
  }
}

inline bool HasTTree(std::filesystem::path filepath,
                     std::string const &treename) {
  struct ROOTFileScopeGuard {
    TDirectory *gold;
    TFile *f;
    ROOTFileScopeGuard(std::filesystem::path path)
        : gold(gDirectory), f{nullptr} {
      auto errlvl = gErrorIgnoreLevel;
      gErrorIgnoreLevel = kFatal;
      f = TFile::Open(path.native().c_str(), "READ");
      gErrorIgnoreLevel = errlvl;
    }
    ~ROOTFileScopeGuard() {
      if (f) {
        f->Close();
        delete f;
      }
      if (gold) {
        gold->cd();
      }
    }
  } rsfg(filepath.native());

  if (!rsfg.f || !rsfg.f->IsOpen() || rsfg.f->IsZombie()) {
    return false;
  }

  auto tt = rsfg.f->Get<TTree>(treename.c_str());
  return bool(tt);
}