#ifndef _ELECTRONICSELEMENTSPAR_H_
#define _ELECTRONICSELEMENTSPAR_H_

#include <string>
#include <vector>
#include <TGraph.h>

#include "ElectronicsElementBase.h"

namespace Electronics
{
	class ElectronicsElementSPar : public ElectronicsElementBase
	{
	public:
		
		// Constructor
		ElectronicsElementSPar(const std::string & file, unsigned int channel, double cutoff = 0);
		ElectronicsElementSPar(const char * file, unsigned int channel, double cutoff = 0);
		
		// Destructor
		virtual ~ElectronicsElementSPar();
		
		virtual void StoreTransferData(TFile & file);

		virtual void ModifyData(Space space, unsigned int size, double * spacere, double * spaceim, double * datare, double * dataim, double scaling);
		
		void SetCutOffFreqency(double freq)
		{
			m_CutOffFreq = freq;
		}
		
	private:
		//init all members
		void Init();
		
		//read in from file (touchstone)
		void Read(const std::string & file, unsigned int channel);
		
		//add data point to s para graphs
		void AddToTGraphs(double freq, double x, double y, bool db = false, bool angle = false);
		
		//reference resistance used for sparameter measurmenet
		double m_ReferenceResitance;
		
		TGraph m_sParaReal; // parameter real
		TGraph m_sParaIm; // parameter img
		
		double m_CutOffFreq; // cutoff freqency
	};
}



#endif