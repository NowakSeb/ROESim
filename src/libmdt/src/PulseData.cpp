#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <vector>
#include <map>
#include <fstream>

#include <TMath.h>
#include <TAxis.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TProfile.h>
#include <TF1.h>
#include <TString.h>
#include <TGraphErrors.h>
#include <TDirectory.h>
#include "PulseData.h"
#include "RTFit.h"

using namespace MDTPulse;
using namespace std;

TGraph * CopyGraph(TGraph * graph)
{
	auto pGraph = new TGraph(graph->GetN());
	
	for(unsigned int i = 0; i < graph->GetN(); i++) {
		double x,y;
		graph->GetPoint(i, x, y);
		pGraph->SetPoint(i, x, y);
	}
	
	return pGraph;
}

double GetMin(TGraph * graph)
{
	double pMin, y;
	graph->GetPoint(0, pMin, y);
	return pMin;
}

double GetMax(TGraph * graph)
{
	double pMax, y;
	graph->GetPoint(graph->GetN()-1, pMax, y);
	return pMax;
}

double GetLength(TGraph * graph)
{
	return GetMax(graph) - GetMin(graph);
}

PulseData::PulseData(std::string file)
{
	m_RT = 0;
	m_TR = 0;
	m_FileName = file;
	m_Debug = false;
	m_File = new TFile(m_FileName.c_str(), "RECREATE");
	
	m_TreeStruct = new TreeStruct();
	m_ResultTree = InitResTree("res", m_TreeStruct);
}

PulseData::~PulseData()
{
	DeleteList<std::list<TGraph *>>(m_PulseList);
	DeleteList<std::vector<TGraph *>>(m_BackgroundList);
	DeleteList<std::list<TGraph *>>(m_CombindedList);
	if (m_RT != 0) {
		m_RT->Delete();
	}
	//destroy file
	m_File->cd();
	m_ResultTree->Write();
	m_ResultTree->Delete();
	m_File->Close();
	m_File->Delete();
	delete m_TreeStruct;
}

TTree * PulseData::InitResTree(TString name, TreeStruct * data)
{
	TTree * tree = new TTree(name,"tree with binary result information");
	tree->Branch("event", &(data->event), "event/I");
	tree->Branch("radius", &(data->radius), "radius/D");
	tree->Branch("time", &(data->time), "time/D");
	tree->Branch("dradius", &(data->dradius), "dradius/D");
	data->event = 0;
	
	return tree;
}


void PulseData::LoadPulsesFromDir(std::string dir, std::string extension, double radius, unsigned int maxfiles)
{
	DeleteList<std::list<TGraph *>>(m_PulseList);
	LoadFromDir<std::list<TGraph *>>(dir, extension, m_PulseList, maxfiles);
	m_TreeStruct->radius = radius;
	m_RadiiList.push_back(radius);
}

void PulseData::LoadPulsesFromRootFile(TFile & file, TString dir, double radius, unsigned int maxfiles)
{
	DeleteList<std::list<TGraph *>>(m_PulseList);
	LoadFromRFile<std::list<TGraph *>>(file, dir, m_PulseList, maxfiles);
	m_TreeStruct->radius = radius;
	m_RadiiList.push_back(radius);
}

void PulseData::CombinePulses(double rate, double noise, unsigned int length)
{
	unsigned int pIndex = 0;
	DeleteList<std::list<TGraph *>>(m_CombindedList);
	for(auto pIt = m_PulseList.begin(); pIt != m_PulseList.end(); pIt++) {
		double pLength = GetLength(*pIt);
		//window -length*pulse until end of pulse
		unsigned int pHits = rate * pLength * (length +1); //average hits in window
		pHits = m_Rand.Poisson(pHits); //now with random number gen
//		auto pGraph = new TGraph(**pIt);
		auto pGraph = CopyGraph(*pIt);
		Extend(pGraph, pLength, noise); //negative time
		//add hits
		for(unsigned int i = 0; i < pHits; i++) {
			unsigned int pBGIndex = m_Rand.Integer(m_BackgroundList.size());
			double pTime = m_Rand.Uniform(-pLength*length, pLength);
			AddGraph(pGraph, m_BackgroundList[pBGIndex], pTime);
		}
		//how many hits in length -> poison
		//uniform random number for times
		m_CombindedList.push_back(pGraph);
		
		pIndex++;
// 		break;
	}
}

void PulseData::WriteData()
{
	WriteList(*m_File, "Pulse", m_PulseList);
	WriteList(*m_File, "Background", m_BackgroundList);
	WriteList(*m_File, "Combinded", m_CombindedList);

	if (m_RT != 0) {
		m_File->cd();
		m_RT->Write("rt");
		m_TR->Write("tr");
	}
}

void PulseData::AddGraph(TGraph * base, TGraph * add, double time)
{
	double y;
	double pBaseMin = GetMin(base);
	double pAddMin = GetMin(add);
	double pAddEnd = GetMax(add);
	
	for(unsigned int i = 0; i < base->GetN(); i++) {
		double pX,pY;
		base->GetPoint(i, pX, pY);
		if (pX < time) {
			continue;
		}
		double pTime = pX - time + pAddMin;
		if (pTime > pAddEnd) {
			break;
		}
		pY += add->Eval(pTime);
		base->SetPoint(i, pX, pY);
	}
	
}

void PulseData::Extend(TGraph * graph, double time, double noise)
{
	double pMin = GetMin(graph);
	double pLength = GetMax(graph) - pMin;
	double pSpacing = pLength / graph->GetN();

	for(unsigned int i = 0; i < graph->GetN(); i++) {
		double pTime, pVal;
		graph->GetPoint(i, pTime, pVal);
		pVal += m_Rand.Gaus(0, noise);
		graph->SetPoint(i, pTime, pVal);
	}
	
	for(double pTime = pMin - time; pTime <= pMin - pSpacing; pTime += pSpacing) {
		double pNoise = m_Rand.Gaus(0, noise);
		graph->SetPoint(graph->GetN(), pTime, pNoise);
	}
	graph->Sort();
}

void PulseData::ApplyElectronics(Electronics::ElectronicsPart & part, TString dir, unsigned int maxfiles, unsigned int mes)
{
	vector<double> pLeading, pTrailing;

	if (!m_File->cd(dir)) {
		m_File->mkdir(dir);
		m_File->cd(dir);
	}
	unsigned int pCounter = 0;
	std::list<TGraph *> * pList = &m_CombindedList;
	if (m_CombindedList.size() == 0) {
		pList = &m_PulseList; //if combined list is empty, use pulse list instead
	}
	for(auto pIt = pList->begin(); pIt != pList->end(); pIt++) {
		TGraph pInput(**pIt);
		part.Process(pInput, 1, false);
		pInput.Write((*pIt)->GetName());
		if (part.BinaryOutput()) {
 			part.GetLatestEdges(pLeading, pTrailing);
			for(unsigned int i = 0; i < pLeading.size(); i++) {
				m_TreeStruct->time = pLeading[i];
				m_TreeStruct->dradius = ConvertRT(pLeading[i]);
				m_ResultTree->Fill();
			}
			m_TreeStruct->event++;
		}
		pCounter++;
		if (maxfiles > 0 && pCounter > 0) {
			if (pCounter % maxfiles == 0) {
				break;
			}
		}
		if (mes > 0 && pCounter > 0) {
			if (pCounter % mes == 0) {
				cout << "\t" << pCounter << " pulses processed..." << endl;
			}
		}
	}
	m_File->cd();
}

void PulseData::ApplyElectronicsOnRootFile(Electronics::ElectronicsPart & part, TFile & file, TString dirin, TString dirout, unsigned int maxfiles, unsigned int mes)
{
	if (!file.cd(dirin)) {
		std::stringstream pStream;
		pStream << "The dir " << dirin << " does not exist in the given root file";
		throw pStream.str();
	}
	vector<TString> pDirsIn;
	vector<TString> pDirsOut;
	vector<double> pRadii;

	TKey * pKey;
	TIter pNextKey (gDirectory->GetListOfKeys());
	while ((pKey = (TKey*) pNextKey()))
	{
		double pRadius;
		pDirsIn.push_back(dirin + "/" + pKey->GetName());
		pDirsOut.push_back(dirout + "/" + pKey->GetName());
		stringstream pStream;
		pStream << pKey->GetName();
		pStream >> pRadius;
		pRadii.push_back(pRadius);
	}

	for(unsigned int i = 0; i < pDirsIn.size(); i++) {
		this->LoadPulsesFromRootFile(file, pDirsIn[i], pRadii[i], mes);
		this->ApplyElectronics(part, pDirsOut[i], maxfiles);
	}
}


void PulseData::SetRTRelation(std::string file)
{
	if (m_RT != 0) {
		m_RT->Delete();
		m_TR->Delete();
	}
	//load rt
	m_RT = new TGraph(file.c_str(), "%lg %lg");
	m_TR = new TGraph(m_RT->GetN());
	
	//generate inverse function
	for(int i= 0; i < m_RT->GetN(); i++) {
		double  x, y;
		m_RT->GetPoint(i, x, y);
		m_TR->SetPoint(i, y, x);
	}
}

double PulseData::ConvertRT(double time)
{
	if (m_RT != 0) {
		return m_RT->Eval(time);
	}
	return 0;
}

void PulseData::CalculateRTRelation(std::string file, double width, unsigned int bins)//min, double max)
{
	ofstream pFile;
	unsigned int pCounter = 0;
	m_RadiiList.sort();
	m_RadiiList.unique();
	
	int iterations = 1;

	m_File->mkdir("rt");
	m_File->cd("rt");
	

// 	for(unsigned int j = 0; j < iterations; j++) {
// 		TString dir = TString::Format("rt/it_%d", j);
// 		m_File->mkdir(dir);
// 		m_File->cd(dir);
// 
// 		for(unsigned int pIndex  = 0; pIndex < m_RadiiList.size(); pIndex++) { // loop over all radii
// 			ROOT::Minuit2::MnUserParameters pUPar;
// 			RTFit pFit(m_ResultTree, m_RT, pUPar, pIndex, 100);
// 			//the fit parameters are set inside the fit class -> makes life easier
// 			
// 			ROOT::Minuit2::MnMigrad pMigrad(pFit, pUPar);
// 			ROOT::Minuit2::FunctionMinimum pMin = pMigrad();
// 			
// //			auto pGraph = new TGraph();
// 			pFit.GetRT(m_RT, pMin);
// 			m_RT->Write(TString::Format("res_%i", pIndex));
// 			
// 			UpdateRTRelation(m_RT);
// 		}
// 
// 		for(auto pIt = m_RadiiList.begin(); pIt != m_RadiiList.end(); pIt++) { // loop over all radii
// 			//per radii
// 			auto pCondition = TString::Format("radius < %f && radius > %f", *pIt + 0.01, *pIt - 0.01);
// 			TString pName = TString::Format("histo_%f", *pIt);
// 			auto pHisto = new TH1F(pName, pName, bins, -1, +1);
// 			TString pCommand = TString::Format("dradius - %f >> %s", *pIt, pName.Data());
// 			m_ResultTree->Draw(pCommand, pCondition);
// 			pHisto->Write(pName);
// 			pHisto->Delete();
// 		}
// 	}
// 	
// 	return;
	
 	auto pFitPoly = new TF1("polyfit", "pol2", 0, 7);//fit function
// 	
// 	int pLength = m_ResultTree->Draw("radius:time", "");
// 	double * pRad = m_ResultTree->GetV1();
// 	double * pTime = m_ResultTree->GetV2();
// 
// 	TProfile * pProfile = new TProfile("prof", "prof", 20, 0, 7);
// 
// 	for(unsigned int i = 0; i < pLength; i++) {
// 		pProfile->Fill(pRad[i], pTime[i]);
// 	}
// 	pProfile->Fit("polyfit", "QR+");
// 	pProfile->Write();
// 	
// 	if (m_TR == 0) {
// 		m_TR = new TGraph(m_RadiiList.size());
// 		m_RT = new TGraph(m_RadiiList.size());
// 	}
// 	
// 	double pStep = 7./m_TR->GetN();
// 	for(unsigned int i = 0; i < m_TR->GetN(); i++) {
// 		m_RT->SetPoint(i, pFitPoly->Eval(i*pStep), i*pStep);
// 		m_TR->SetPoint(i, i*pStep, pFitPoly->Eval(i*pStep));
// 	}
// 	UpdateRTRelation(m_RT);
// // 	return;

	if (m_TR == 0) {
		m_TR = new TGraph(m_RadiiList.size());
		m_RT = new TGraph(m_RadiiList.size());
	}
	else{
		m_TR->Set(0);
		m_RT->Set(0);
	}
	ofstream pRTFile(file);
	for(auto pIt = m_RadiiList.begin(); pIt != m_RadiiList.end(); pIt++) { // loop over all radii
		auto pCondition = TString::Format("radius < %f && radius > %f", *pIt + 0.01, *pIt - 0.01);
		//all
		//per radii
		TString pName = TString::Format("histo_%f", *pIt);
//		auto pHisto = new TH1F(pName, pName, bins, -pWidth[*pIt], +pWidth[*pIt]);
		int pLength = m_ResultTree->Draw("time", pCondition);
		double * pTime = m_ResultTree->GetV1();
		double pMean = 0;
		for(unsigned int i = 0; i < pLength; i++) {
			pMean += pTime[i];
		}
		pMean /= pLength;
		
		m_RT->SetPoint(m_RT->GetN(), pMean, *pIt);
		m_TR->SetPoint(m_TR->GetN(), *pIt, pMean);
		pRTFile << pMean << "\t" << *pIt << endl;
	}
	pRTFile.close();
	m_RT->Write("rt");
	m_TR->Write("tr");
	UpdateRTRelation(m_RT);
	
	return;
	
	//prepare width
	std::map<double, double> pWidth;
	for(auto pIt = m_RadiiList.begin(); pIt != m_RadiiList.end(); pIt++) { // loop over all radii
		pWidth[*pIt] = width;
	}

	pFitPoly->Delete();
	pFitPoly = new TF1("polyfit", "pol5", 0, 7);//fit function
	
	
	for(unsigned int j = 0; j < iterations; j++) {
		TString dir = TString::Format("rt/it_%d", j);
		m_File->mkdir(dir);
		m_File->cd(dir);
		
		auto pGraph = new TGraph();
		
		for(auto pIt = m_RadiiList.begin(); pIt != m_RadiiList.end(); pIt++) { // loop over all radii
			auto pFitFunction = new TF1("resfit", "gaus",-pWidth[*pIt], +pWidth[*pIt]);//fit function
			auto pCondition = TString::Format("radius < %f && radius > %f", *pIt + 0.01, *pIt - 0.01);
			//all
			//per radii
			TString pName = TString::Format("histo_%f", *pIt);
			auto pHisto = new TH1F(pName, pName, bins, -pWidth[*pIt], +pWidth[*pIt]);
			TString pCommand = TString::Format("dradius - %f >> %s", *pIt, pName.Data());
			m_ResultTree->Draw(pCommand, pCondition);
			pFitFunction->SetParameter(1, pHisto->GetMean());
			pFitFunction->SetParameter(2, pHisto->GetRMS());
			pHisto->Fit("resfit", "QR+");
			pHisto->Write(pName);
			pHisto->Delete();
			
			double pTime = m_TR->Eval(*pIt + 0.5*pFitFunction->GetParameter(1));
			pWidth[*pIt] = 5 * pFitFunction->GetParameter(2);
			//pWidth += 5 * pFitFunction->GetParameter(2);
 			cout << *pIt << " " << pFitFunction->GetParameter(1) << endl;
			pGraph->SetPoint(pGraph->GetN(), *pIt, pTime);
			pFitFunction->Delete(); //not needed anymore
		}
		
		pGraph->Sort();
		pGraph->Fit(pFitPoly, "Q");
		pGraph->Write("rt");

		if (j == iterations - 1) {
			//open in last iteration
			pFile.open(file);
		}
		
		for(int i = 0; i < pGraph->GetN(); i++) {
			double x, y;
			pGraph->GetPoint(i, x, y);
			if (j == iterations - 1) {
				//last iteration -> go to file
				pFile << pFitPoly->Eval(x) <<"\t" << x << endl;
			}
 			pGraph->SetPoint(i, pFitPoly->Eval(x), x);
// 			pGraph->SetPoint(i, y, x);
		}
		pGraph->Write("tr");
		UpdateRTRelation(pGraph);
		
		pGraph->Delete();
	}
	pFile.close();
	
/*
	
	
	for(auto pIt = m_RadiiList.begin(); pIt != m_RadiiList.end(); pIt++) {
		TString pName = TString::Format("%.1f", *pIt);
		auto * pHisto = new TH1F("histo", "histo", 1000, min, max);
		auto pString = TString::Format("radius < %f && radius > %f && time > %f && time < %f", *pIt + 0.01, *pIt - 0.01, min, max);
		m_ResultTree->Draw("time>>histo", pString);
		auto pFitFunction = new TF1("rtfit", "gaus");//, pHisto->GetMean() -0.2, pHisto->GetMean() + 0.2);
		pFitFunction->SetParameter(1, pHisto->GetMean());
		pFitFunction->SetParameter(2, pHisto->GetRMS());
		pHisto->Fit("rtfit", "QL");
		auto pFitFunctionEx = new TF1("rtfit1", "rtfit", pFitFunction->GetParameter(1) - 0.2, pFitFunction->GetParameter(1) + 0.2);
		pHisto->Fit("rtfit1", "QR");
		pFile << pFitFunction->GetParameter(1) << "\t" << *pIt << endl;
		pGraph->SetPoint(pGraph->GetN(), pFitFunctionEx->GetParameter(1), *pIt);
// 		pGraph->SetPoint(pGraph->GetN(), pHisto->GetMean(), *pIt);
		pHisto->Write(pName);
		pHisto->Delete();
		pFitFunction->Delete();
		pFitFunctionEx->Delete();
	}*/
}

double PulseData::GetResolution(double width, unsigned int bins)
{
	unsigned int pCounter = 0;
	m_RadiiList.sort();
	m_RadiiList.unique();

	TString dir = "Resolution";
	m_File->mkdir(dir);
	m_File->cd(dir);
	
	TGraphErrors * pResGraph = new TGraphErrors(m_RadiiList.size());
	TGraphErrors * pResAll = new TGraphErrors(1);
	auto pFitFunction = new TF1("resfit", "gaus",-width, +width);//fit function
	auto pHistoAll = new TH1F("histo_all", "histo", 3*bins, -width, +width); //histogram for all radii
	for(auto pIt = m_RadiiList.begin(); pIt != m_RadiiList.end(); pIt++) { // loop over all radii
		auto pCondition = TString::Format("radius < %f && radius > %f", *pIt + 0.01, *pIt - 0.01);
		//all
		auto pCommand = TString::Format("dradius-%f>>+histo_all", *pIt);
		m_ResultTree->Draw(pCommand, pCondition);
		//per radii
		TString pName = TString::Format("histo_%f", *pIt);
		auto pHisto = new TH1F(pName, pName, bins, -width, +width);
		pCommand = TString::Format("dradius - %f >> %s", *pIt, pName.Data());
		m_ResultTree->Draw(pCommand, pCondition);
		pFitFunction->SetParameter(1, pHisto->GetMean());
		pFitFunction->SetParameter(2, pHisto->GetRMS());
		pHisto->Fit("resfit", "Q");
		pResGraph->SetPoint(pCounter, *pIt, pFitFunction->GetParameter(2));
		pResGraph->SetPointError(pCounter, pFitFunction->GetParError(2));
		
		pHisto->Write();
		pHisto->Delete();

		pCounter++;
	}
	pFitFunction->SetParameter(1, pHistoAll->GetMean());
	pFitFunction->SetParameter(2, pHistoAll->GetRMS());
	pHistoAll->Fit("resfit", "Q");
	pResAll->SetPoint(0, 0, pFitFunction->GetParameter(2));
	pResAll->SetPointError(0, pFitFunction->GetParError(2));

	double pRes = pFitFunction->GetParameter(2);
	
	cout << "Resolution: " << pRes << " (" << pFitFunction->GetParError(2) << ") mm" << endl;
	
	pHistoAll->Write();
	pResGraph->Write("res_detail");
	pResAll->Write("res_all");
	
	pResGraph->Delete();
	pResAll->Delete();
	pHistoAll->Delete();
	pFitFunction->Delete();
	return pRes;
}

// double PulseData::GetEff3(double res)
// {
// 	
// }

void PulseData::UpdateRTRelation(TGraph * rt)
{
	TreeStruct * pData = new TreeStruct();
	TTree * pTree = InitResTree("res_int", pData);
	
	for (int i = 0, N = m_ResultTree->GetEntries(); i < N; i++) {
		m_ResultTree->GetEntry(i);
		*pData = *m_TreeStruct;
		pData->dradius = rt->Eval(pData->time);
		pTree->Fill();
	}
// 	m_ResultTree->Delete();
	m_ResultTree = pTree;
	m_ResultTree->SetName("res");
	delete m_TreeStruct;
	m_TreeStruct = pData;
}

