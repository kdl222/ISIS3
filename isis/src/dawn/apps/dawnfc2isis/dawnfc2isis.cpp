#include "Isis.h"

#include <cstdio>
#include <string>

#include "ProcessImportPds.h"
#include "ProcessBySample.h"
#include "UserInterface.h"
#include "Filename.h"

using namespace std;
using namespace Isis;

void flipbyline(Buffer &in, Buffer &out);

void IsisMain() {
  ProcessImportPds p;
  Pvl pdsLabel;
  UserInterface &ui = Application::GetUserInterface();

  Filename inFile = ui.GetFilename("FROM");
  iString instid;
  iString missid;

  try {
    Pvl lab(inFile.Expanded());
    instid = (string) lab.FindKeyword("INSTRUMENT_ID");
    missid = (string) lab.FindKeyword("MISSION_ID");
  }
  catch(iException &e) {
    string msg = "Unable to read [INSTRUMENT_ID] or [MISSION_ID] from input file [" +
                 inFile.Expanded() + "]";
    throw iException::Message(iException::Io, msg, _FILEINFO_);
  }

  instid.ConvertWhiteSpace();
  instid.Compress();
  instid.Trim(" ");
  missid.ConvertWhiteSpace();
  missid.Compress();
  missid.Trim(" ");
  if(missid != "DAWN" && instid != "FC1" && instid != "FC2") {
    string msg = "Input file [" + inFile.Expanded() + "] does not appear to be " +
                 "a DAWN Framing Camera (FC) EDR or RDR file.";
    throw iException::Message(iException::Io, msg, _FILEINFO_);
  }

  std::string target;
  if(ui.WasEntered("TARGET")) {
    target = ui.GetString("TARGET");
  }


  p.SetPdsFile(inFile.Expanded(), "", pdsLabel);
  p.SetOrganization(Isis::ProcessImport::BSQ);
  string tmpName = "$TEMPORARY/" + inFile.Basename() + ".tmp.cub";
  Filename tmpFile(tmpName);
  CubeAttributeOutput outatt = CubeAttributeOutput("+Real");
  p.SetOutputCube(tmpFile.Expanded(), outatt);
  p.SaveFileHeader();

  Pvl labelPvl(inFile.Expanded());

  p.StartProcess();
  p.EndProcess();

  ProcessBySample p2;
  CubeAttributeInput inatt;
  p2.SetInputCube(tmpFile.Expanded(), inatt);
  Cube *outcube = p2.SetOutputCube("TO");

  // Get the directory where the DAWN translation tables are.
  PvlGroup dataDir(Preference::Preferences().FindGroup("DataDirectory"));
  iString transDir = (string) dataDir["Dawn"] + "/translations/";

  // Create a PVL to store the translated labels in
  Pvl outLabel;

  // Translate the BandBin group
  Filename transFile(transDir + "dawnfcBandBin.trn");
  PvlTranslationManager bandBinXlater(labelPvl, transFile.Expanded());
  bandBinXlater.Auto(outLabel);

  // Translate the Archive group
  transFile = transDir + "dawnfcArchive.trn";
  PvlTranslationManager archiveXlater(labelPvl, transFile.Expanded());
  archiveXlater.Auto(outLabel);

  // Translate the Instrument group
  transFile = transDir + "dawnfcInstrument.trn";
  PvlTranslationManager instrumentXlater(labelPvl, transFile.Expanded());
  instrumentXlater.Auto(outLabel);

  //  Update target if user specifies it
  if (!target.empty()) {
    PvlGroup &igrp = outLabel.FindGroup("Instrument",Pvl::Traverse);
    igrp["TargetName"] = iString(target);
  }

  // Write the BandBin, Archive, and Instrument groups
  // to the output cube label
  outcube->PutGroup(outLabel.FindGroup("BandBin", Pvl::Traverse));
  outcube->PutGroup(outLabel.FindGroup("Archive", Pvl::Traverse));
  outcube->PutGroup(outLabel.FindGroup("Instrument", Pvl::Traverse));

  // Set the BandBin filter name, center, and width values based on the
  // FilterNumber.
  PvlGroup &bbGrp(outLabel.FindGroup("BandBin", Pvl::Traverse));
  int filtno = bbGrp["FilterNumber"];
  int center;
  int width;
  string filtname;
  if(filtno == 1) {
    center = 700;
    width = 700;
    filtname = "Clear_F1";
  }
  else if(filtno == 2) {
    center = 555;
    width = 43;
    filtname = "Green_F2";
  }
  else if(filtno == 3) {
    center = 749;
    width = 44;
    filtname = "Red_F3";
  }
  else if(filtno == 4) {
    center = 917;
    width = 45;
    filtname = "NIR_F4";
  }
  else if(filtno == 5) {
    center = 965;
    width = 85;
    filtname = "NIR_F5";
  }
  else if(filtno == 6) {
    center = 829;
    width = 33;
    filtname = "NIR_F6";
  }
  else if(filtno == 7) {
    center = 653;
    width = 42;
    filtname = "Red_F7";
  }
  else if(filtno == 8) {
    center = 438;
    width = 40;
    filtname = "Blue_F8";
  }
  else {
    string msg = "Input file [" + inFile.Expanded() + "] has an invalid " +
                 "FilterNumber. The FilterNumber must fall in the range 1 to 8.";
    throw iException::Message(iException::Io, msg, _FILEINFO_);
  }
  bbGrp.AddKeyword(PvlKeyword("Center", center));
  bbGrp.AddKeyword(PvlKeyword("Width", width));
  bbGrp.AddKeyword(PvlKeyword("FilterName", filtname));
  outcube->PutGroup(bbGrp);

  PvlGroup kerns("Kernels");
  if(instid == "FC1") {
    kerns += PvlKeyword("NaifFrameCode", -203110);
  }
  else if(instid == "FC2") {
    kerns += PvlKeyword("NaifFrameCode", -203120);
  }
  else {
    string msg = "Input file [" + inFile.Expanded() + "] has an invalid " +
                 "InstrumentId.";
    throw iException::Message(iException::Io, msg, _FILEINFO_);
  }
  outcube->PutGroup(kerns);

  p2.StartProcess(flipbyline);
  p2.EndProcess();

  string tmp(tmpFile.Expanded());
  remove(tmp.c_str());
}

// Flip image by line
void flipbyline(Buffer &in, Buffer &out) {
  int index = in.size() - 1;
  for(int i = 0; i < in.size(); i++) {
    out[i] = in[index - i];
  }
}
