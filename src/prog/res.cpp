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

int main(int argc, char* argv[])
{
	vector<string> pDirs;
	vector<double> pRadii;
	ElectronicsPart pPart;
	
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
		string pRTFile;
		if (pASD) {
			pRTFile = "../../data/rt_ASD.txt";
		}
		else {
			pRTFile = "../../data/rt_BLR.txt";
		}
		
		//pulse data
		TFile pInput(pNameIn.c_str(), "READ");
		
		PulseData pData(pNameOut, pInput);

		pData.SetRTRelation(pRTFile);
		if (pRT) {
			pData.CalculateRTRelation(pRTFile, 4.95e-6, 5.2e-6);//4.95e-6, 5.2e-6
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
