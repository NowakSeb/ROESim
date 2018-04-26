#include <string>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <vector>
#include <fstream>

#include <TMath.h>
#include <TAxis.h>

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
	m_FileName = file;
	m_Debug = false;
	m_File = new TFile(m_FileName.c_str(), "RECREATE");
}

PulseData::~PulseData()
{
	DeleteList<std::list<TGraph *>>(m_PulseList);
	DeleteList<std::vector<TGraph *>>(m_BackgroundList);
	DeleteList<std::list<TGraph *>>(m_CombindedList);
	//destroy file
	m_File->Close();
	m_File->Delete();
}


void PulseData::CombinePulses(double rate, double noise, unsigned int length)
{
	unsigned int pIndex = 0;
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
	m_File->mkdir(dir.c_str());
	m_File->cd(dir.c_str());
	for(auto pIt = m_CombindedList.begin(); pIt != m_CombindedList.end(); pIt++) {
		TGraph pInput(**pIt);
		part.Process(pInput, 1, false);
		pInput.Write((*pIt)->GetName());
	}
	m_File->cd();
}
