#include <iostream>
#include <TFile.h>
#include <TMath.h>
#include "ElectronicsPart.h"

using namespace std;
using namespace Electronics;

void FillGraph(TGraph & data, TGraph & data1)
{
	double x[] = {0,1,2,3,4,5,6,7,8,9,10,11};
// 	double y1[] = {-1,1,-1,1,-1,1,-1,1,-1,1,-1,1};
	double y1[] = {-1,0,1,0,-1,0,1,0,-1,0,1,0};
	double y2[] = {0,0,0,0,0,0,0,0,0,0,0};//TMath::Pi()/2
	
	data = TGraph(10,x,y1);
	data1 = TGraph(10,x,y2);
}

int main()
{
	TFile pFile("test.root", "RECREATE");
	//ElectronicsElementSPar pSPar("test.s2p", 1);
	//pSPar.StoreTransferData(pFile);
// 	TGraph pGraph, pGraph1;
// 	FillGraph(pGraph, pGraph1);
// 	pFile.cd();
// 	pGraph.Write("orig");
	try {
		ElectronicsPart pPart;
		pPart.AddElementSParameter("../../testdata/ASD.s2p",1, 0);
		pPart.StoreTransferData(pFile);
		TGraph pDRes = pPart.GetDeltaResponse(1e-6, 1e-9);
		pDRes.Write("dres");
		// 		ElectronicsElementData pData(pGraph, 1);
// 		ElectronicsElementData pData(pGraph, pGraph1, false);
// 		pData.SetLog(true);
// 		pData.SetSpace(Electronics::Space::FOURIER);
// 		pData.GetDataTGraph(pGraph, pGraph1);
// 		pGraph.Write("real");
// 		pGraph1.Write("im");
//  		pData.SetSpace(Electronics::Space::TIME);
//   		pGraph = pData.GetDataTGraph();
//   		pGraph.Write("trans");
		
		
// 		pData.SetSampling(0.5);
// 		TGraph pGraph2 = pData.GetDataTGraph();
// 		cout << pGraph2.GetN() << " points" << endl;
// 		cout << pData.GetSampling() << " " << pData.GetTicksInSeconds() << endl;
	}
	catch (std::string ex) {
		cout << ":Err " << ex << endl;
	}
	
	pFile.cd();
// 	pGraph.Write("data");
	return 0;
}