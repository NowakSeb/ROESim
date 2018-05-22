#ifndef _PULSEDATA_H_
#define _PULSEDATA_H_

#include <fstream>
#include <list>
#include <vector>
#include <TFile.h>
#include <TGraph.h>
#include <TRandom3.h>
#include <TSystemDirectory.h>
#include <TList.h>
#include <TTree.h>
#include "ElectronicsPart.h"

namespace MDTPulse
{
	class PulseData
	{
	public:
		
		// Constructor
		PulseData(std::string file);
		// Destructor
		virtual ~PulseData();
		
		//load pulse data, all old pulses are deleted
		void LoadPulsesFromDir(std::string dir, std::string extension, double radius);
		
		//load background data
		void LoadBackgroundFromDir(std::string dir, std::string extension)
		{
			LoadFromDir<std::vector<TGraph *>>(dir, extension, m_BackgroundList);
		}
		
		//scale pulse x and y dimension
		void ScalePulses(double x, double y)
		{
			ApplyScaling<std::list<TGraph *>>(x, y, m_PulseList);
		}

		//scale background pulse x and y dimension
		void ScaleBackground(double x, double y)
		{
			ApplyScaling<std::vector<TGraph *>>(x, y, m_BackgroundList);
		}

		//multiples of pulse length
		void CombinePulses(double rate, double noise = 0, unsigned int length = 5);
		
		void SetDebug(bool state)
		{
			m_Debug = state;
		}

		//process and stor data in root file
		void ApplyElectronics(Electronics::ElectronicsPart & part, std::string dir);
		
		//store input and background data in root file
		void WriteData();

		//return graph with time as a function of the radius (rt-relation)
		void StoreTimeRadiusGraph(std::string file);

		//return graph with time as a function of the radius (rt-relation)
		void SetRTRelation(std::string file);
		
		//return average resolution
		double GetResolution(double width, unsigned int bins);
		
	private:
		
		struct TreeStruct {
			unsigned int event;
			double radius; // actual radius
			double time; // drift time
			double dradius; //converted radius
		};
		
		//random number calc of the instance
		TRandom3 m_Rand;
		
		//corresponding root file
		std::string m_FileName;
		
		//root file
		TFile * m_File;
		
		//store intermediate data
		bool m_Debug;
		
		//signal pulses
		std::list<TGraph *> m_PulseList;

		//background pulses
		std::vector<TGraph *> m_BackgroundList;

		//combinded pulses
		std::list<TGraph *> m_CombindedList;

		//tree with information necessary for resolution and efficiency calc
		TTree * m_ResultTree;

		//support struct for m_ResultTree
		TreeStruct m_TreeStruct;
		
		//list with all applied radii
		std::list<double> m_RadiiList;
		
		//TGraph with rt relation
		TGraph * m_RT;
		
		//add second graph on top of first
		static void AddGraph(TGraph * base, TGraph * add, double time);
		
		//add baseline in front of graph
		void Extend(TGraph * graph, double time, double noise = 0);
		
		//rt conversion
		double ConvertRT(double time);
		
		//scales the graphs in the given list
		template< class container >
		void ApplyScaling(double x, double y, container & pulselist)
		{
			for(auto pIt = pulselist.begin(); pIt != pulselist.end(); pIt++) {
				int i;
				for(i = 0; i < (*pIt)->GetN(); i++) {
					double pX, pY;
					(*pIt)->GetPoint(i, pX, pY);
					(*pIt)->SetPoint(i, pX*x, pY*y);
				}
			}
		}
		

		//loads all files from a given dir and stores in the given list
		template< class container >
		void LoadFromDir(std::string dir, std::string extension, container & pulselist)
		{
			TSystemDirectory pDir(dir.c_str(), dir.c_str());
			TList * pFileList = pDir.GetListOfFiles();
			if (pFileList)
			{
				TSystemFile * pFile;
				TString pName;
				TIter next(pFileList);
				while ((pFile=(TSystemFile*)next()))
				{
					pName = pFile->GetName();
					if (!pFile->IsDirectory() && pName.EndsWith(extension.c_str()))
					{
						std::string pFullName = dir + "/" + pName.Data();
						
						auto pGraph = new TGraph(pFullName.c_str());
						pulselist.push_back(pGraph);
					}
				}
			}
		}
		
		//writes all graphs in the given list to a dir
		template< class container >
		void WriteList(TFile & file, std::string dir, container & pulselist)
		{
			unsigned int pIndex = 0;
			file.mkdir(dir.c_str());
			file.cd(dir.c_str());
		
			for(auto pIt = pulselist.begin(); pIt != pulselist.end(); pIt++) {
				(*pIt)->Write(TString::Format("Pulse_%i", pIndex));
				pIndex++;
			}
		}
		
		//delete all elements of this list and clear it afterwards
		template< class container >
		void DeleteList(container & pulselist)
		{
			for(auto pIt = pulselist.begin(); pIt != pulselist.end(); pIt++) {
				(*pIt)->Delete();
			}
			pulselist.clear();
		}
	};
}



#endif