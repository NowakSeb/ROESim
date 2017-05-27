#include <string>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <vector>
#include <fstream>

#include <TMath.h>

#include "ElectronicsPart.h"
#include "ElectronicsElementData.h"

using namespace Electronics;
using namespace std;

ElectronicsPart::ElectronicsPart(Precision precision) : m_Precision(precision)
{
	m_Debug = false;
}

ElectronicsPart::~ElectronicsPart()
{
	Clear();
}

void ElectronicsPart::Process(TGraph & input, double ticks, bool sort)
{
	list<Electronics::ElectronicsElementBase *>::iterator pIt;
	ElectronicsElementData pData(input, ticks, m_Precision, sort);
	
	pData.SetLog(true);

// 	apply elements
	for(pIt = m_Elements.begin(); pIt != m_Elements.end(); pIt++) {
		pData.ApplyElement(**pIt);
	}
	
	//and get final output
	pData.SetSpace(Space::FOURIER);
	pData.SetSpace(Space::TIME);
 	m_DataGraph = pData.GetDataTGraph();
	input = m_DataGraph;
}

void ElectronicsPart::AddElement(Electronics::ElectronicsElementBase * element)
{
	element->SetDebug(m_Debug);
	m_Elements.push_back(element);
}

void ElectronicsPart::StoreTransferData(TFile & file)
{
	list<Electronics::ElectronicsElementBase *>::iterator pIt;
	for(pIt = m_Elements.begin(); pIt != m_Elements.end(); pIt++) {
		(*pIt)->StoreTransferData(file);
	}
}

void ElectronicsPart::Clear()
{
	list<Electronics::ElectronicsElementBase *>::iterator pIt;
	for(pIt = m_Elements.begin(); pIt != m_Elements.end(); pIt++) {
		delete *pIt;
	}
	m_Elements.clear();
}

TGraph & ElectronicsPart::GetDeltaResponse(double length, double spacing)
{
	TGraph pInput(3);
	pInput.SetPoint(0,0,1);
	pInput.SetPoint(1,spacing,0);
	pInput.SetPoint(2,length,0);
	
	m_DataGraph = pInput;

	Process(pInput);
	return m_DataGraph;
}

void ElectronicsPart::SetDebug(bool set)
{
	list<Electronics::ElectronicsElementBase *>::iterator pIt;
	for(pIt = m_Elements.begin(); pIt != m_Elements.end(); pIt++) {
		(*pIt)->SetDebug(set);
	}
	m_Debug = set;
}

