#include <string>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <vector>
#include <fstream>

#include <TMath.h>

#include "ElectronicsElementDiscriminator.h"

using namespace Electronics;
using namespace std;

ElectronicsElementDiscriminator::ElectronicsElementDiscriminator(double threshold, double low, double high, double hysteresis)
{
	m_SpaceCapability.push_back(Space::TIME);
	m_BinaryOutput = true;
	
	m_Threshold = threshold;
	m_LevelLow = low;
	m_LevelHigh = high;
	m_Hysteresis = hysteresis;
}

ElectronicsElementDiscriminator::~ElectronicsElementDiscriminator()
{
}

void ElectronicsElementDiscriminator::StoreTransferData(TFile & file)
{
// 	file.cd();
// 	file.mkdir(m_Name.c_str());
// 	file.cd(m_Name.c_str());
// 	m_sParaReal.Write("real");
// 	m_sParaIm.Write("im");
// 	file.cd();
}

void ElectronicsElementDiscriminator::ModifyData(Space space, unsigned int & size, double * & spacere, double * & spaceim, double * & datare, double * & dataim, double scaling)
{
	if (space != Space::TIME) {
		throw string("Data needs to be in time space in order to discriminate.");
	}
// 	for(unsigned int i = 0; i < size; i++) {
// 		double pFreq = scaling * spacere[i];
// 		if (pFreq > m_CutOffFreq && m_CutOffFreq > 0) {
// 			//higher frequencies are ignored
// 			break;
// 		}
// 		double pScatRe = m_sParaReal.Eval(pFreq);
// 		double pScatIm = m_sParaIm.Eval(pFreq);
// 		
// 		double pReal = pScatRe * datare[i] - pScatIm * dataim[i];
// 		double pIm = pScatRe * dataim[i] + pScatIm * datare[i];
// 		datare[i] = pReal;
// 		dataim[i] = pIm;
// 	}
// 	
}
