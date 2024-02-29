#include "nuis/eventinput/EventSourceFactory.h"

#include "nuis/weightcalc/WeightCalcFactory.h"

#include "nuis/frame/FrameGen.h"
#include "nuis/histframe/HistFrame.h"
#include "nuis/histframe/ROOTUtility.h"
#include "nuis/histframe/Utility.h"

#include "NuHepMC/EventUtils.hxx"
#include "NuHepMC/FATXUtils.hxx"
#include "NuHepMC/ReaderUtils.hxx"

#include "TCanvas.h"
#include "ProSelecta/ProSelecta.h"

#include "spdlog/spdlog.h"

#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wnarrowing"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

using namespace nuis;

using jitfunc = int (*)(int argc, char const *argv[]);

int main(int argc, char const *argv[]) {
  
  // Grab Files
  std::string file_to_read = std::string(argv[1]);
  std::string function_name = "main";

  ProSelecta::Get().AddIncludePath("/usr/include/"); 
  ProSelecta::Get().AddIncludePath("/opt/HepMC3/git_master/include/"); 
  ProSelecta::Get().AddIncludePath("/opt/NuHepMC_CPPUtils/git_master/include/"); 
  ProSelecta::Get().AddIncludePath("/opt/ProSelecta/git_master/include/"); 
  ProSelecta::Get().AddIncludePath("/opt/genie/3_04_00/include/"); 
  ProSelecta::Get().AddIncludePath("/opt/lhapdf/5.9.1/include/"); 
  ProSelecta::Get().AddIncludePath("/opt/neut/git_master/include/"); 
  ProSelecta::Get().AddIncludePath("/opt/pybind11/v2.9.2/include/"); 
  ProSelecta::Get().AddIncludePath("/opt/root/v6-28-08/include/"); 
  ProSelecta::Get().AddIncludePath("/opt/yaml-cpp/v0.8.0/include/"); 

  // Add this for now as a testing env to allow JIT C++ Compiling
  // This is just a poor mans root interpreter with all the
  // nuisance linked headers already good to go
  ProSelecta::Get().LoadFile(file_to_read.c_str());

  // Call the function based on assumed void arglist
  auto func = ProSelecta::Get().FindClingSym<jitfunc>(function_name,
    "int argc, char const *argv[]");
  

  // return result
  return func(argc, argv);
}