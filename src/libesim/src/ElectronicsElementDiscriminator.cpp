#include <string>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <vector>
#include <fstream>

#include <TMath.h>
#include <TGraph.h>

#include "ElectronicsElementDiscriminator.h"

using namespace Electronics;
using namespace std;

ElectronicsElementDiscriminator::ElectronicsElementDiscriminator(double threshold, double low, double high, double hysteresis, double deadtime, double risetime)
{
	m_SpaceCapability.push_back(Space::TIME);
	m_BinaryOutput = true;
	
	m_Threshold = threshold;
	m_LevelLow = low;
	m_LevelHigh = high;
	m_Hysteresis = hysteresis;
	m_DeadTime = deadtime;
	m_RiseTime = risetime;
}

ElectronicsElementDiscriminator::~ElectronicsElementDiscriminator()
{
}

void ElectronicsElementDiscriminator::StoreTransferData(TFile & file)
{
}

void ElectronicsElementDiscriminator::ModifyData(Space space, unsigned int & size, double * & spacere, double * & spaceim, double * & datare, double * & dataim, double scaling)
{
	if (space != Space::TIME) {
		throw string("Data needs to be in time space in order to discriminate.");
	}
	TGraph pGraph;

	//no hysteresis implemented so far
	double pLastLeading = spacere[0] - m_DeadTime / scaling;
	bool pActive = false;
	for(unsigned int i = 0; i < size; i++) {
		double pLeadDiff = spacere[i] - pLastLeading;
		bool pTriggered = false;
		//is signal high enough to trigger
		if (datare[i] > m_Threshold && m_Threshold > 0) {
			//positive signs
			pTriggered = true;
		}
		else if (datare[i] < m_Threshold && m_Threshold < 0) {
			//negative signs
			pTriggered = true;
		}
		
		//include deadtime
		if (pTriggered && !pActive && pLeadDiff * scaling >= m_DeadTime) {
			pGraph.SetPoint(pGraph.GetN(), spacere[i], m_LevelLow);
			pGraph.SetPoint(pGraph.GetN(), spacere[i] + m_RiseTime, m_LevelHigh);
			pLastLeading = spacere[i];
			pActive = true;
		}
		else if (!pTriggered && pActive){
			pGraph.SetPoint(pGraph.GetN(), spacere[i], m_LevelHigh);
			pGraph.SetPoint(pGraph.GetN(), spacere[i] + m_RiseTime, m_LevelLow);
			pActive = false;
		}
		
	}
	if (!m_BinaryOnly) {
		//update datare, etc..
	}
	
	pGraph.Write();
}
