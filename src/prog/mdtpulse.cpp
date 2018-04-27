#include <iostream>
#include <TFile.h>
#include <TMath.h>
#include "PulseData.h"
#include "ElectronicsPart.h"

using namespace std;
using namespace MDTPulse;
using namespace Electronics;

int main()
{
	try {
		PulseData pData("test.root");
		pData.LoadPulsesFromDir("/home/snowak/MPI/Electronics2018/ROESim/testdata/garfield_pulse/3.5", "sig");
		pData.LoadBackgroundFromDir("/home/snowak/MPI/Electronics2018/ROESim/testdata/gamma_pulse", "txt");
		//now data is us and uA -> use SI here
		pData.ScalePulses(1e-6, 1e-6);
		pData.ScaleBackground(1, -1.0e-5*2);
		pData.CombinePulses(400e3, 0.5e-7);
		
		ElectronicsPart pPart;
		pPart.SetDebug();
		//pPart.AddElementSpice("../../testdata/spice_rc.txt", "N000", "N001");
		pPart.AddElementSParameter("../../testdata/ASD.s2p",1, 0);
		pPart.AddElementDiscriminator(-0.1, 0, 1, 0, 0.1e-6);
		pData.ApplyElectronics(pPart, "ASD");
		
// 		pDRes.Write("dres0");
// 		pDRes = pPart.GetDeltaResponse(1e-7, 1e-8);
// 		pDRes.Write("dres1");
		
		
		
		pData.WriteData();
		
	
	}
	catch (std::string ex) {
		cout << ":Err " << ex << endl;
	}

// 	pGraph.Write("data");
	return 0;
}