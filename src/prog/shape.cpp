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
	bool pASD = true;
	
	if (argc != 4) {
		cout << "3 Arguments are obligatory: <input> <output> <ASD or BLR>" << endl;
		return - 1;
	}
	string pNameIn = argv[1];
	string pNameOut = argv[2];
	
	if (string(argv[3]) == "ASD") {
		pASD = true;
	}
	else if (string(argv[3]) == "BLR") {
		pASD = false;
	}
	else {
		cout << "Argument 3 has to be either ASD or BLR. " << argv[3] << " is not valid." << endl;
		return -2;
	}
	
	cout << "Input: " << pNameIn << "; Output: " << pNameOut << "; Type: " << argv[3] << endl; 
	
	try {
		//prepare part
		ElectronicsPart pPart;
		if (pASD) {
			PrepareCircuitASD(pPart);
		}
		else {
			PrepareCircuitVBLR(pPart);
		}
		
		//pulse data
		PulseData pData(pNameOut);

		string pRTFile = "/home/snowak/MPI/Electronics2018/ROESim/testdata/garfield_pulse_nodelta/rt.txt";
 		pData.SetRTRelation(pRTFile);
		
		//loop over dirs
		TFile pInput(pNameIn.c_str());
		
		if(pASD) {
			pData.ApplyElectronicsOnRootFile(pPart, pInput, "raw", "ASD");
		}
		else {
			pData.ApplyElectronicsOnRootFile(pPart, pInput, "raw", "BLR");
		}
		
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
