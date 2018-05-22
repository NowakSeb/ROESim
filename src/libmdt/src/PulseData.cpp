#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <vector>
#include <fstream>

#include <TMath.h>
#include <TAxis.h>
#include <TH1F.h>
#include <TString.h>

#include "PulseData.h"

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
	m_FileName = file;
	m_Debug = false;
	m_File = new TFile(m_FileName.c_str(), "RECREATE");
	m_ResultTree = new TTree("res","tree with binary result information");
	m_ResultTree->Branch("event", &(m_TreeStruct.event), "event/I");
	m_ResultTree->Branch("radius", &(m_TreeStruct.radius), "radius/D");
	m_ResultTree->Branch("time", &(m_TreeStruct.time), "time/D");
	m_ResultTree->Branch("dradius", &(m_TreeStruct.dradius), "dradius/D");
	m_TreeStruct.event = 0;
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
}

void PulseData::LoadPulsesFromDir(std::string dir, std::string extension, double radius)
{
	DeleteList<std::list<TGraph *>>(m_PulseList);
	LoadFromDir<std::list<TGraph *>>(dir, extension, m_PulseList);
	m_TreeStruct.radius = radius;
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
	
	for(double pTime = pMin - time; pTime <= pMin - pSpacing; pTime += pSpacing) {
		double pNoise = m_Rand.Gaus(0, noise);
		graph->SetPoint(graph->GetN(), pTime, pNoise);
	}
	graph->Sort();
}

void PulseData::ApplyElectronics(Electronics::ElectronicsPart & part, std::string dir)
{
	vector<double> pLeading, pTrailing;

	if (!m_File->cd(dir.c_str())) {
		m_File->mkdir(dir.c_str());
		m_File->cd(dir.c_str());
	}
	for(auto pIt = m_CombindedList.begin(); pIt != m_CombindedList.end(); pIt++) {
		TGraph pInput(**pIt);
		part.Process(pInput, 1, false);
		pInput.Write((*pIt)->GetName());
		if (part.BinaryOutput()) {
			part.GetLatestEdges(pLeading, pTrailing);
			for(unsigned int i = 0; i < pLeading.size(); i++) {
				m_TreeStruct.time = pLeading[i];
				m_TreeStruct.dradius = ConvertRT(pLeading[i]);
				m_ResultTree->Fill();
			}
			m_TreeStruct.event++;
		}
	}
	m_File->cd();
}

void PulseData::SetRTRelation(std::string file)
{
	if (m_RT != 0) {
		m_RT->Delete();
	}
	m_RT = new TGraph(file.c_str(), "%lg %lg");
}

double PulseData::ConvertRT(double time)
{
	if (m_RT != 0) {
		return m_RT->Eval(time);
	}
	return 0;
}

void PulseData::StoreTimeRadiusGraph(std::string file)
{
	ofstream pFile(file);
	m_RadiiList.sort();
	m_RadiiList.unique();
	
	for(auto pIt = m_RadiiList.begin(); pIt != m_RadiiList.end(); pIt++) {
		auto * pHisto = new TH1F("histo", "histo", 1, 0, 1);
		auto pString = TString::Format("radius < %f && radius > %f", *pIt + 0.01, *pIt - 0.01);
		m_ResultTree->Draw("time>>histo", pString);
		pFile << pHisto->GetMean() << "\t" << *pIt << endl;
		pHisto->Delete();
	}
	pFile.close();
}

double PulseData::GetResolution(double width, unsigned int bins)
{
	m_RadiiList.sort();
	m_RadiiList.unique();

	TString dir = "Resolution";
	m_File->mkdir(dir);
	m_File->cd(dir);
	
	auto * pHistoAll = new TH1F("histo_all", "histo", bins, -width, +width);
	for(auto pIt = m_RadiiList.begin(); pIt != m_RadiiList.end(); pIt++) {
		auto pCondition = TString::Format("radius < %f && radius > %f", *pIt + 0.01, *pIt - 0.01);
		//all
		auto pCommand = TString::Format("dradius-%f>>+histo_all", *pIt);
		m_ResultTree->Draw(pCommand, pCondition);
		//per radii
		auto pName = TString::Format("histo_%f", *pIt);
		auto * pHisto = new TH1F(pName, pName, bins, -width, +width);
		pCommand = TString::Format("dradius-%f>>%s", *pIt, pName);
		m_ResultTree->Draw(pCommand, pCondition);
		pHisto->Write();
//		pHisto->Delete();
	}
	pHistoAll->Write();
}
