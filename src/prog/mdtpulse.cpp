#include <list>
#include <string>
#include <iostream>
#include <TFile.h>
#include <TMath.h>
#include "PulseData.h"
#include "ElectronicsPart.h"

using namespace std;
using namespace MDTPulse;
using namespace Electronics;

void PrepareCircuit(ElectronicsPart & part);

int main()
{
	vector<string> pDirs;
	vector<double> pRadii;
	
	try {
		//prepare part
		ElectronicsPart pPart;
		PrepareCircuit(pPart);

		//pulse data
		PulseData pData("test.root");
		string pRTFile = "/home/snowak/MPI/Electronics2018/ROESim/testdata/garfield_pulse/rt.txt";

		pDirs.push_back("/home/snowak/MPI/Electronics2018/ROESim/testdata/garfield_pulse/0.5");
		pRadii.push_back(0.5);
		pDirs.push_back("/home/snowak/MPI/Electronics2018/ROESim/testdata/garfield_pulse/1.5");
		pRadii.push_back(1.5);
		pDirs.push_back("/home/snowak/MPI/Electronics2018/ROESim/testdata/garfield_pulse/2.5");
		pRadii.push_back(2.5);
		pDirs.push_back("/home/snowak/MPI/Electronics2018/ROESim/testdata/garfield_pulse/3.5");
		pRadii.push_back(3.5);
		pDirs.push_back("/home/snowak/MPI/Electronics2018/ROESim/testdata/garfield_pulse/4.5");
		pRadii.push_back(4.5);
		pDirs.push_back("/home/snowak/MPI/Electronics2018/ROESim/testdata/garfield_pulse/5.5");
		pRadii.push_back(5.5);
		pDirs.push_back("/home/snowak/MPI/Electronics2018/ROESim/testdata/garfield_pulse/6.5");
		pRadii.push_back(6.5);
		
		//prepare bg
		pData.LoadBackgroundFromDir("/home/snowak/MPI/Electronics2018/ROESim/testdata/gamma_pulse", "txt");
		pData.ScaleBackground(1, -1.0e-5*2);
		//now data is us and uA -> use SI here

		pData.SetRTRelation(pRTFile);

		//loop over dirs
		for(unsigned int i = 0; i < pDirs.size(); i++) {
			pData.LoadPulsesFromDir(pDirs[i], "sig", pRadii[i]);
			//now data is us and uA -> use SI here
			pData.ScalePulses(1e-6, 1e-6);
			pData.CombinePulses(0e3, 0.0e-7);
			pData.ApplyElectronics(pPart, "ASD");
		}
		
		//Get rt
 		//pData.StoreTimeRadiusGraph(pRTFile);
		
		//calculate resolution
		pData.GetResolution(1.0, 50);
		
		pData.WriteData();
		
	
	}
	catch (std::string ex) {
		cout << ":Err " << ex << endl;
	}

// 	pGraph.Write("data");
	return 0;
}

void PrepareCircuit(ElectronicsPart & part)
{
	part.SetDebug();
	//pPart.AddElementSpice("../../testdata/spice_rc.txt", "N000", "N001");
	part.AddElementSParameter("../../testdata/ASD.s2p",1, 0);
	part.AddElementDiscriminator(-0.034, 0, 1, 0, 0.1e-6);
}
