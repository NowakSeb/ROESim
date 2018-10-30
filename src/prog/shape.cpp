#include <list>
#include <string>
#include <iostream>
#include <sstream>
#include <TFile.h>
#include <TMath.h>
#include "PulseData.h"
#include "ElectronicsPart.h"

using namespace std;
using namespace MDTPulse;
using namespace Electronics;

void PrepareCircuitASD(ElectronicsPart & part);
void PrepareCircuitVBLR(ElectronicsPart & part);

int main(int argc, char* argv[])
{
	vector<string> pDirs;
	vector<double> pRadii;
// 
	try {
		//prepare part
		ElectronicsPart pPartASD;
		PrepareCircuitASD(pPartASD);
		ElectronicsPart pPartBLR;
		PrepareCircuitVBLR(pPartBLR);
		
		string pOutASD = "ASD_shaped_" + string(argv[1]) + ".root";
		string pOutBLR = "BLR_shaped_" + string(argv[1]) + ".root";
		
		//pulse data
		PulseData pDataASD(pOutASD);
		PulseData pDataBLR(pOutBLR);
		string pRTFile = "/home/snowak/MPI/Electronics2018/ROESim/testdata/garfield_pulse_nodelta/rt.txt";

 		pDataASD.SetRTRelation(pRTFile);
		
		//loop over dirs
		TString pIn = TString::Format("raw_%s.root", argv[1]);
		TFile pInput(pIn);
		
//		pDataASD.ApplyElectronicsOnRootFile(pPartASD, pInput, "raw", "ASD");
		
		pDataBLR.ApplyElectronicsOnRootFile(pPartBLR, pInput, "raw", "BLR",100);
	}
	catch (std::string ex) {
		cout << ":Err " << ex << endl;
	}

// 	pGraph.Write("data");
	return 0;
}

void PrepareCircuitVBLR(ElectronicsPart & part)
{
	string pPath = "wine /home/snowak/.wine/drive_c/Program\\ Files/LTC/LTspiceXVII/XVIIx64.exe";
//	string pFile = "../../spice/AnalogChannelLTSpice/AnalogChannelCurrentSource_x2.asc";
	string pFile = "AnalogChannelCurrentSource_x2.asc";
	part.SetDebug();
 	part.AddElementAmplifier(10000.);
	part.AddElementLTSpice(pPath, pFile, "data.sig", "vo");//tp4
// 	part.AddElementDiscriminator(-0.02, 0, 1, 0, 20.e-6); //-0.05 is correct
}


void PrepareCircuitASD(ElectronicsPart & part)
{
	part.SetDebug();
	//pPart.AddElementSpice("../../testdata/spice_rc.txt", "N000", "N001");
	part.AddElementSParameter("../../testdata/ASD.s2p",1, 0);
	part.AddElementDiscriminator(-0.05, 0, 1, 0, 100.e-6); //-0.05 is correct
}
