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

void PrepareCircuit(ElectronicsPart & part);
void PrepareCircuitDis(ElectronicsPart & part);

int main(int argc, char* argv[])
{
	vector<string> pDirs;
	vector<double> pRadii;
	
	if (argc !=3) {
		cout << "2 Arguments are obligatory: <BG Rate in Hz> <filename (.root is added)>." << endl;
		return - 1;
	}
	double pRate;
	string pName = argv[2] + string(".root");
	stringstream pStream(argv[1]);
	pStream >> pRate;
	
	cout << "File " << pName << " with " << pRate << " Hz background." << endl; 
	
	try {
		//prepare part
		ElectronicsPart pPart;
		PrepareCircuitDis(pPart);
		
		//pulse data
		PulseData pData(pName);
		// 		PulseData pData("raw_input.root");
		
		pDirs.push_back("/home/snowak/MPI/Electronics2018/ROESim/testdata/garfield_pulse/0.5");
		pRadii.push_back(0.5);
		pDirs.push_back("/home/snowak/MPI/Electronics2018/ROESim/testdata/garfield_pulse/1");
		pRadii.push_back(1);
		pDirs.push_back("/home/snowak/MPI/Electronics2018/ROESim/testdata/garfield_pulse/1.5");
		pRadii.push_back(1.5);
		pDirs.push_back("/home/snowak/MPI/Electronics2018/ROESim/testdata/garfield_pulse/2");
		pRadii.push_back(2);
		pDirs.push_back("/home/snowak/MPI/Electronics2018/ROESim/testdata/garfield_pulse/2.5");
		pRadii.push_back(2.5);
		pDirs.push_back("/home/snowak/MPI/Electronics2018/ROESim/testdata/garfield_pulse/3");
		pRadii.push_back(3);
		pDirs.push_back("/home/snowak/MPI/Electronics2018/ROESim/testdata/garfield_pulse/3.5");
		pRadii.push_back(3.5);
		pDirs.push_back("/home/snowak/MPI/Electronics2018/ROESim/testdata/garfield_pulse/4");
		pRadii.push_back(4);
		pDirs.push_back("/home/snowak/MPI/Electronics2018/ROESim/testdata/garfield_pulse/4.5");
		pRadii.push_back(4.5);
		pDirs.push_back("/home/snowak/MPI/Electronics2018/ROESim/testdata/garfield_pulse/5");
		pRadii.push_back(5);
		pDirs.push_back("/home/snowak/MPI/Electronics2018/ROESim/testdata/garfield_pulse/5.5");
		pRadii.push_back(5.5);
		pDirs.push_back("/home/snowak/MPI/Electronics2018/ROESim/testdata/garfield_pulse/6");
		pRadii.push_back(6);
		pDirs.push_back("/home/snowak/MPI/Electronics2018/ROESim/testdata/garfield_pulse/6.5");
		pRadii.push_back(6.5);
		
		//prepare bg
		pData.LoadBackgroundFromDir("/home/snowak/MPI/Electronics2018/ROESim/testdata/gamma_pulse", "txt");
		pData.ScaleBackground(1, -1./10000/2.);
		//now data is us and uA -> use SI here
		
		pData.WriteData();
		
		//loop over dirs
		for(unsigned int i = 0; i < pDirs.size(); i++) {
			pData.LoadPulsesFromDir(pDirs[i], "sig", pRadii[i], 500);
			//now data is us and uA -> use SI here
			stringstream pStream;
			pStream << "raw/" << pRadii[i];
			pData.ScalePulses(1e-9, 1e-6);
			pData.CombinePulses(pRate, 10e-9, 2, pStream.str().c_str());
		}
		
	}
	catch (std::string ex) {
		cout << ":Err " << ex << endl;
	}
}

void PrepareCircuitDis(ElectronicsPart & part)
{
	part.SetDebug();
	// 	part.AddElementDiscriminator(-0.002, 0, 1, 0, 20.e-6); //-0.05 is correct
	part.AddElementDiscriminator(1.06, 0, 1, 0, 20.e-6); //-0.05 is correct
}
