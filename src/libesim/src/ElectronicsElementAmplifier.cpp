#include <string>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <vector>
#include <fstream>

#include <TMath.h>
#include <TGraph.h>

#include "ElectronicsElementAmplifier.h"

using namespace Electronics;
using namespace std;

ElectronicsElementAmplifier::ElectronicsElementAmplifier(double gain, double bandwidth)
{
	m_SpaceCapability.push_back(Space::TIME);
	m_BinaryOutput = false;
	
	m_Gain = gain;
	m_Bandwidth = bandwidth;
}

ElectronicsElementAmplifier::~ElectronicsElementAmplifier()
{
}

void ElectronicsElementAmplifier::StoreTransferData(TFile & file)
{
}

void ElectronicsElementAmplifier::ModifyData(Space space, unsigned int & size, double * & spacere, double * & spaceim, double * & datare, double * & dataim, double scaling)
{
	if (space != Space::TIME && m_Bandwidth != 0) {
		throw string("Data needs to be in fourier space in order to amplify with bandwidth.");
	}

	//bandwidth not implemented so far!!!
	for(unsigned int i = 0; i < size; i++) {
		datare[i] = datare[i] * m_Gain;
	}

	if (space == Space::FOURIER) {
		for(unsigned int i = 0; i < size; i++) {
			dataim[i] = dataim[i] * m_Gain;
		}
	}
}
