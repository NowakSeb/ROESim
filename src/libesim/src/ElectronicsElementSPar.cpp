#include <string>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <vector>
#include <fstream>

#include <TMath.h>

#include "ElectronicsElementSPar.h"

using namespace Electronics;
using namespace std;

void StringToUpper(string & dat)
{
	std::transform(dat.begin(), dat.end(),dat.begin(), ::toupper);
}

double dB2Gain(double db)
{
	return TMath::Power(10., db / 20.);
}


double FreqToNum(string & dat)
{
	StringToUpper(dat);
	if (dat == "HZ")
		return 1;
	else if (dat == "KHZ")
		return 1e3;
	else if  (dat == "MHZ")
		return 1e6;
	else if  (dat == "GHZ")
		return 1e9;
	return 0;
}

unsigned int CountColumns(const string & data)
{
	stringstream pStream;
	string pBuf;
	unsigned int pCount = 0;
	pStream << data;
	
	while(!pStream.eof()) {
		pStream >> pBuf;
		pCount++;
	}
	return pCount;
}

void GetColumnData(const string & input, double & freq, double & first, double & second, unsigned int column)
{
	stringstream pStream;
	double pBuf;
	
	if (input == "") {
		freq = 0;
		return;
	}
	pStream << input;
	pStream >> freq;
	for(unsigned int i = 0; !pStream.eof() && pStream.good(); i+=2) {
		if (2*column == i){
			pStream >> first >> second;
		}
		else {
			pStream >> pBuf >> pBuf;
		}
	}
}

void ElectronicsElementSPar::AddToTGraphs(double freq, double x, double y, bool db, bool angle)
{
	if (db) {
		x = dB2Gain(x);
	}
	if (angle) {
		double re, im;
		re = x * TMath::Cos(y*TMath::Pi()/180.);
		im = x * TMath::Sin(y*TMath::Pi()/180.);
		x = re;
		y = im;
	}
	m_sParaReal.SetPoint(m_sParaReal.GetN(), freq, x);
	m_sParaIm.SetPoint(m_sParaIm.GetN(), freq, y);
}

ElectronicsElementSPar::ElectronicsElementSPar(const std::string & file, unsigned int channel, double cutoff)
{
	m_CutOffFreq = cutoff;
	Init();
	Read(file, channel);
}

ElectronicsElementSPar::ElectronicsElementSPar(const char * file, unsigned int channel, double cutoff)
{
	m_CutOffFreq = cutoff;
	Init();
	SetName(file);
	Read(string(file), channel);
}

ElectronicsElementSPar::~ElectronicsElementSPar()
{
}

void ElectronicsElementSPar::Init()
{
	m_ConstSampling = true; //elemnt accepts everything
	//accepts only fourier space data
	m_SpaceCapability.push_back(Space::FOURIER);
	m_ReferenceResitance = 50.0; //50 ohm reference resistance per default
	m_CutOffFreq = 0; //not cutoff frequency per default
}

void ElectronicsElementSPar::StoreTransferData(TFile & file)
{
	file.cd();
	file.mkdir(m_Name.c_str());
	file.cd(m_Name.c_str());
	m_sParaReal.Write("real");
	m_sParaIm.Write("im");
	file.cd();
}

void ElectronicsElementSPar::Read(const std::string & file, unsigned int channel)
{
	ifstream pFile;
	
	pFile.open(file.c_str());

	if (!pFile.good()) {
		stringstream pStream;
		pStream << "Cannot open " << file << ". Does this file exist?"; 
		throw pStream.str();
	}
	
	string pLimiter, pFreq, pType, pFormat;
	
	//read options
	do {
		stringstream pLine;
		pLine << ReadLine(pFile);
		pLine >> pLimiter >> pFreq >> pType >> pFormat >> m_ReferenceResitance;
	} while (pFile.good() && pLimiter != "#");

	// <FREQ_UNITS> = Units of the frequency data. Options are GHz, MHz, KHz, or Hz.
	//	<TYPE> = Type of file data. Options are:  S, Y or Z for S1P components, S, Y, Z, G, or H for S2P components, S for 3 or more ports
	//	<FORMAT> = S-parameter format. Options are: DB for dB-angle, MA for magnitude angle, RI for real-imaginary
	// <Rn> = Reference resistance in ohms, where n is a positive number. This is the impedance the S-parameters were normalized to.
	
	//interpret them
	double pFreqFactor = FreqToNum(pFreq);

	//count lines to determine order of pole
	string pLine1 = NextLine(pFile);
	string pLine2 = NextLine(pFile);
	unsigned int pCount1 = CountColumns(pLine1);
	unsigned int pCount2 = CountColumns(pLine2);
	
	//reset graphs
	m_sParaReal.Set(0);
	m_sParaIm.Set(0);
	
	//1 and 2 pole
	if (pCount1 == pCount2 && pCount1 > 0) {
		double pFreq, pVal1, pVal2;
		bool pDB = false;
		bool pAngle = false;
		//set options
		if (pFormat == "DB") {
			pDB = true;
			pAngle = true;
		}
		else if (pFormat == "MA") {
			pAngle = true;
		}
		//process first two lines
		GetColumnData(pLine1, pFreq, pVal1, pVal2, channel);
 		AddToTGraphs(pFreq * pFreqFactor, pVal1, pVal2, pDB, pAngle);
		GetColumnData(pLine2, pFreq, pVal1, pVal2, channel);
		AddToTGraphs(pFreq * pFreqFactor, pVal1, pVal2, pDB, pAngle);

		//normal read
		while(pFile.good()) {
			GetColumnData(NextLine(pFile), pFreq, pVal1, pVal2, channel);
			if (pFreq == 0) {
				//either end of file or something is fishy...
				break;
			}
//  			/*cout*/ << pFreq << " " << pFreqFactor << endl;
			AddToTGraphs(pFreq * pFreqFactor, pVal1, pVal2, pDB, pAngle);
		}
		
	}
	else  {
		//check for other poles -> don't care at the moment
		//still to do...
	}
}

// void ElectronicsElementSPar::ModifyData(Space space, unsigned int size, double * spacere, double * spaceim, double * datare, double * dataim, double scaling)
void ElectronicsElementSPar::ModifyData(Space space, unsigned int & size, double * & spacere, double * & spaceim, double * & datare, double * & dataim, double scaling)
{
	if (space != Space::FOURIER) {
		throw string("Data needs to be in Fourier space in order to apply s-parameters.");
	}
	for(unsigned int i = 0; i < size; i++) {
		double pFreq = scaling * spacere[i];
		if (pFreq > m_CutOffFreq && m_CutOffFreq > 0) {
			//higher frequencies are ignored
			break;
		}
		double pScatRe = m_sParaReal.Eval(pFreq);
		double pScatIm = m_sParaIm.Eval(pFreq);
		
		double pReal = pScatRe * datare[i] - pScatIm * dataim[i];
		double pIm = pScatRe * dataim[i] + pScatIm * datare[i];
		datare[i] = pReal;
		dataim[i] = pIm;
	}
	
}
