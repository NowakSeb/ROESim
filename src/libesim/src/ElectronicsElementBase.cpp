#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include "ElectronicsElementBase.h"


using namespace Electronics;
using namespace std;

ElectronicsElementBase::ElectronicsElementBase()
{
	m_ConstSampling = true;
	m_Debug = false;
	m_BinaryOutput = false;
	m_BinaryOnly = false;
}

ElectronicsElementBase::~ElectronicsElementBase()
{

}

string ElectronicsElementBase::ReadLine(ifstream & file)
{
	char pChar;
	string pLine;
	
	while(file.good())
	{
		pChar = file.get();
		if (pChar == '\n')
		{
			return pLine;
		}
		else if (pChar == '\r')
		{
			continue;
		}
		pLine.append(1, pChar);
		
	}
	return pLine;
}

string ElectronicsElementBase::NextLine(ifstream & file, char comment)
{
	string pLine;
	
	while(!file.eof())
	{
		pLine = ReadLine(file);
		if (pLine[0] == comment)
		{
			continue;
		}
		else if (pLine == "")
		{
			continue;
		}
		else if (pLine.size() < 2) {
			continue;
		}
		return pLine;
	}
	return "";
}

bool ElectronicsElementBase::CapableOfSpace(Electronics::Space space)
{
	for(unsigned int i = 0; i < m_SpaceCapability.size(); i++) {
		if (m_SpaceCapability[i] == space) {
			return true;
		}
	}
	return false;
}

void ElectronicsElementBase::SetDebug(bool set)
{
	m_Debug = set;
}

void ElectronicsElementBase::GetEdges(std::vector<double> & leading, std::vector<double> & trailing)
{
	leading.clear();
	trailing.clear();
	for(unsigned int i = 0; i < m_EdgeTrailing.size(); i++) {
		leading.push_back(m_EdgeLeading[i]);
		trailing.push_back(m_EdgeTrailing[i]);
	}
	if (m_EdgeTrailing.size() < m_EdgeLeading.size()) {
		leading.push_back(m_EdgeLeading[m_EdgeLeading.size()-1]);
	}
}
