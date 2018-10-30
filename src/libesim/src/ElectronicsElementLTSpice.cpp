#include <string>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <vector>
#include <fstream>

#include <TSystem.h>


#include "ElectronicsElementLTSpice.h"

using namespace Electronics;
using namespace std;

std::string const ElectronicsElementLTSpice::m_IntFile = "IntLTSpice.asc";
std::string const ElectronicsElementLTSpice::m_ResFile = "IntLTSpice.raw";

ElectronicsElementLTSpice::ElectronicsElementLTSpice(const std::string & path, const std::string & file, const std::string & input, const std::string & output)
{
	SetName(file);
	//spice takes also non const sampling everything
	m_ConstSampling = false;
	//accepts only time space data
	m_SpaceCapability.push_back(Space::TIME);
	//input, output
	m_InputFile = input;
	m_OutputNet = output;
	m_LTSpicePath = path;
	
	//prepare file
	ifstream pInStream(file);
	if (!pInStream.good()){
		stringstream pStream;
		pStream << "Cannot open " << file << ". Does this file exist?"; 
		throw pStream.str();
	}
	ofstream pOutStream(m_IntFile);
	
	while(pInStream.good()) {
		char pBuffer[1024];
		pInStream.getline(pBuffer, 1024);
		if (sizeof(pBuffer) == 0) {
			continue;
		}
		pOutStream << pBuffer << endl;
	}
	pOutStream << "TEXT 704 1672 Left 2 !.save " << output << endl;
	pInStream.close();
	pOutStream.close();
}

ElectronicsElementLTSpice::~ElectronicsElementLTSpice()
{
	
}

void ElectronicsElementLTSpice::StoreTransferData(TFile & file)
{
	
}

void ElectronicsElementLTSpice::ModifyData(Space space, unsigned int & size, double * & spacere, double * & spaceim, double * & datare, double * & dataim, double scaling)
{
	ofstream pFile(m_InputFile);
	
	//write data
	for(unsigned int i = 0; i < size; i++) {
		pFile << spacere[i]*scaling << "\t" << datare[i] << endl; 
	}
	pFile.close();
	
	//process
	TString pCommand = TString::Format("%s -ascii -b -Run %s > out.log 2> err.log", m_LTSpicePath.c_str(), m_IntFile.c_str());
// 	gSystem->Exec(pCommand);
	gSystem->GetFromPipe(pCommand);
	
	//read back and update spacere, datare
	ReadRawFile(m_ResFile, spacere, datare, size, scaling);
}

void ElectronicsElementLTSpice::SetDebug(bool set)
{
	ElectronicsElementBase::SetDebug(set);
}

void ElectronicsElementLTSpice::ReadRawFile(const std::string & filename, double * & time, double * & data, unsigned int & length, double scaling)
{
	unsigned int pVariables = 0;
	unsigned int pPoints = 0;
	
	ifstream pInput(filename);
	
	if (!pInput.good()) {
		stringstream pStream;
		pStream << "LTSpice output file " << m_ResFile << " cannot be opened. Does it exist? Check for errors.";
		throw pStream.str();
	}
	
	//read header
	for(unsigned int pLine = 0; pLine < 10; pLine++) {
		char pBuffer[1024];
		pInput.getline(pBuffer, 1024);
		string pString(pBuffer);
		if (pString.find("No. Variables:") == 0) {
			stringstream pStream;
			pStream << pString;
			pStream >> pString >> pString >> pVariables;
		}
		else if (pString.find("No. Points:") == 0) {
			stringstream pStream;
			pStream << pString;
			pStream >> pString >> pString >> pPoints;
		}
	}
	if (pVariables > 2) {
		stringstream pStream;
		pStream << " The LTSpice raw file contains " << pVariables << " variales, but only 2 are expected. Sometings fishy...";
		throw pStream.str();
	}

	//prepare pointers
	delete time;
	delete data;
	
	time = new double[pPoints];
	data = new double[pPoints];
	length = pPoints;
	
	//ignore all lines inbetween
	bool pDataSection = false;
	while(pInput.good()) {
		char pBuffer[1024];
		pInput.getline(pBuffer, 1024);
		string pString(pBuffer);
		if (sizeof(pBuffer) == 0) {
			continue;
		}
		if (pString.find("Values:") == 0) {
			pDataSection = true;
			break;
		}
	}
	if (!pDataSection) {
		throw string("Cannot find data section in LTSpice raw file. Sometings fishy...");
	}
	
	//loop over result lines
	pPoints = 0;
	while(pInput.good()) {
		unsigned int pIndex;
		pInput >> pIndex >> time[pPoints] >> data[pPoints];
		time[pPoints] /= scaling;
// 		cout << pIndex << "\t" << time[pPoints] << "\t" << data[pPoints] << endl;
		pPoints++;
		if (pPoints == length) {
			break;
		}
	}
	
	pInput.close();
}
