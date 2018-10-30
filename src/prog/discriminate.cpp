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

void PrepareCircuitASD(ElectronicsPart & part, bool rt);
void PrepareCircuitVBLR(ElectronicsPart & part, bool rt);

int main(int argc, char* argv[])
{
	vector<string> pDirs;
	vector<double> pRadii;
// 
	bool pASD = true;
	bool pRT = false;
	
	if (argc != 4) {
		cout << "3 Arguments are obligatory: <input> <output> <ASD or BLR>" << endl;
		return - 1;
	}
	string pNameIn = argv[1];
	string pNameOut = argv[2];
	
	if (string(argv[3]) == "ASD") {
		pASD = true;
		pRT = false;
	}
	else if (string(argv[3]) == "BLR") {
		pASD = false;
		pRT = false;
	}
	else if (string(argv[3]) == "ASDrt") {
		pASD = true;
		pRT = true;
	}
	else if (string(argv[3]) == "BLRrt") {
		pASD = false;
		pRT = true;
	}
	else {
		cout << "Argument 3 has to be either ASD or BLR. " << argv[3] << " is not valid." << endl;
		return -2;
	}
	
	cout << "Input: " << pNameIn << "; Output: " << pNameOut << "; Type: " << argv[3] << endl; 
	
	
	try {
		//prepare part
		ElectronicsPart pPart;
		string pRTFile;
		if (pASD) {
			PrepareCircuitASD(pPart, pRT);
			pRTFile = "../../data/rt_ASD.txt";
		}
		else {
			PrepareCircuitVBLR(pPart, pRT);
			pRTFile = "../../data/rt_BLR.txt";
		}
		
		//pulse data
		PulseData pData(pNameOut);

		pData.SetRTRelation(pRTFile);
		
		//loop over dirs
		TFile pInput(pNameIn.c_str());
		if (pASD) {
			pData.ApplyElectronicsOnRootFile(pPart, pInput, "ASD", "ASD");
		}
		else {
			pData.ApplyElectronicsOnRootFile(pPart, pInput, "BLR", "BLR");
		}
		if (pRT) {
			pData.CalculateRTRelation(pRTFile);
		}

		//calculate resolution
 		double pRes = pData.GetResolution(1.0, 75);
		double pEffTotal = pData.GetEfficiency();
		double pEff3 = pData.GetEfficiency(3*pRes);
		
		cout << pRes << "\t" << pEffTotal << "\t" << pEff3 << endl;
	}
	catch (std::string ex) {
		cout << ":Err " << ex << endl;
	}

// 	pGraph.Write("data");
	return 0;
}

void PrepareCircuitVBLR(ElectronicsPart & part, bool rt)
{
	if (rt) {
		part.AddElementDiscriminator(1, 0, 1, 0, 100.e-6); //-0.05 is correct
	}
	else {
		part.AddElementDiscriminator(1, 0, 1, 0, 100.e-9); //-0.05 is correct
	}
}


void PrepareCircuitASD(ElectronicsPart & part, bool rt)
{
	if (rt) {
		part.AddElementDiscriminator(-0.05, 0, 1, 0, 100.e-6);
	}
	else {
		part.AddElementDiscriminator(-0.05, 0, 1, 0, 100.e-9);
	}
}
