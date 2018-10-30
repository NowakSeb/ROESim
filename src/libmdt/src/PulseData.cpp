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

PulseData::PulseData(std::string output, TFile & input)
{
	m_RT = 0;
	m_TR = 0;
	m_FileName = output;
	m_Debug = false;
	m_TreeStruct = new TreeStruct();
	
	input.cd();
	TTree * pTree = dynamic_cast<TTree *> (input.Get("res"));
	
	if (pTree == 0) {
		throw string("Root file does not conatin result tree");
	}
	
	m_File = new TFile(m_FileName.c_str(), "RECREATE");
	m_File->cd();

	m_ResultTree = pTree->CloneTree();
	
	m_ResultTree->GetBranch("event")->SetAddress(&(m_TreeStruct->event));
	m_ResultTree->GetBranch("radius")->SetAddress(&(m_TreeStruct->radius));
	m_ResultTree->GetBranch("time")->SetAddress(&(m_TreeStruct->time));
	m_ResultTree->GetBranch("dradius")->SetAddress(&(m_TreeStruct->dradius));
	
	//get radii list
	int pLen = m_ResultTree->Draw("radius","");
	m_RadiiList = list<double>(m_ResultTree->GetV1(), m_ResultTree->GetV1() + pLen);
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
//	m_ResultTree->Delete();
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

void PulseData::CombinePulses(double rate, double noise, unsigned int length, TString dir)
{
	unsigned int pIndex = 0;
	DeleteList<std::list<TGraph *>>(m_CombindedList);
	for(auto pIt = m_PulseList.begin(); pIt != m_PulseList.end(); pIt++) {
		double pLength = GetLength(*pIt);
		//window -length*pulse until end of pulse
		double pHitsAVG = rate * pLength * (length +1); //average hits in window
		unsigned int pHits = m_Rand.Poisson(pHitsAVG); //now with random number gen
		//		auto pGraph = new TGraph(**pIt);
		auto pGraph = CopyGraph(*pIt);
		Extend(pGraph, length * pLength, noise); //negative time
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
	
	if (dir != "") {
		m_File->cd();
		m_File->mkdir(dir);
		m_File->cd(dir);
		unsigned int pIndex = 0;
		for(auto pIt = m_CombindedList.begin(); pIt != m_CombindedList.end(); pIt++) {
			(*pIt)->Write(TString::Format("Pulse_%i", pIndex));
			pIndex++;
		}
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
	
	if (m_ResultTree != 0) {
		UpdateRTRelation(m_RT);
	}
}

double PulseData::ConvertRT(double time)
{
	if (m_RT != 0) {
		return m_RT->Eval(time);
	}
	return 0;
}

void PulseData::CalculateRTRelation(const std::string & file, double min, double max)
{
	ofstream pFile;
	unsigned int pCounter = 0;
	m_RadiiList.sort();
	m_RadiiList.unique();
	
	int iterations = 1;

	m_File->mkdir("rt");
	m_File->cd("rt");
	
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
		TString pCondition = TString::Format("radius < %f && radius > %f", *pIt + 0.01, *pIt - 0.01);
		if (min != 0 || max != 0) {
			pCondition += TString::Format(" && time < %f*1e-6 && time > %f*1e-6", max*1e6, min*1e6);
		}
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

double PulseData::GetEfficiency(double width)
{
	auto pHistoAll = new TH1F("histo_all", "histo", 1, -width, +width); //histogram for all radii
	auto pHistoSelect = new TH1F("histo_sel", "histo", 1, -width, +width); //histogram for all radii
}


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

