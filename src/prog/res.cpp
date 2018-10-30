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
		ElectronicsPart pPart;
		PrepareCircuitASD(pPart);

		
		//pulse data
		string pOutASD = "ASD_" + string(argv[1]) + ".root";
		string pOutBLR = "BLR_" + string(argv[1]) + ".root";
		PulseData pData(pOutASD);
		string pRTFile = "rt_ASD.txt";

 		pData.SetRTRelation(pRTFile);
		
		//loop over dirs
		TString pInASD = TString::Format("raw_%s.root", argv[1]);
// 		TString pInASD = TString::Format("raw_%s.root", argv[1]);
		TFile pInput(pInASD);
		pData.ApplyElectronicsOnRootFile(pPart, pInput, "raw", "ASD");
		

		//calculate resolution
 		double pRes = pData.GetResolution(1.0, 75);
// 		double pEff = pData.GetEfficiency(3*pRes);
		
		pData.WriteData();
		
	
	}
	catch (std::string ex) {
		cout << ":Err " << ex << endl;
	}

// 	pGraph.Write("data");
	return 0;
}

void PrepareCircuitVBLR(ElectronicsPart & part)
{
 	part.AddElementDiscriminator(1, 0, 1, 0, 100.e-9); //-0.05 is correct
}


void PrepareCircuitASD(ElectronicsPart & part)
{
	part.AddElementDiscriminator(-0.05, 0, 1, 0, 100.e-9);
}
