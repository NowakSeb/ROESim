#ifndef _ELECTRONICSPART_H_
#define _ELECTRONICSPART_H_

#include <vector>
#include <list>

//#include "ElectronicsElementBase.h"
#include "ElectronicsElementData.h"
#include "ElectronicsElementSPar.h"

namespace Electronics
{
	class ElectronicsPart
	{
	public:
		
		ElectronicsPart(Precision precision = Precision::FULL);

		// Destructor
		virtual ~ElectronicsPart();
		
		/**
		 * add s-parameter element
		 * @param file filename 
		 * @param channel of pole
		 * @param cutoff frequency
		 */
		void AddElementSParameter(const std::string & file, unsigned int channel, double cutoff = 0)
		{
			AddElement(new ElectronicsElementSPar(file, channel, cutoff));
		}
	
		/**
		 * @param input data in time space
		 * @param ticks time unit in seconds
		 * @param precision calculation precision used
		 * @param sort if true, data is sorted in timespace
		 */
		void Process(TGraph & input, double ticks = 1, bool sort = false);
		
		/**
		 * Get Delta response for this element 
		 * @param length total length of signal, delta input is at 0
		 * @param number of data points used for calculation
		 */
		TGraph & GetDeltaResponse(double length=1, double spacing=1e-3);
		
		/**
		 * stores all available transfer data to file
		 */
		void StoreTransferData(TFile & file);
		
		//clear instance, remove all elements
		void Clear();
		
	protected:
		
		void AddElement(ElectronicsElementBase * element);

		//list of all elements
		std::list<ElectronicsElementBase *> m_Elements;
		
		//precision used for calculation
		Electronics::Precision m_Precision;
		
		//instance used for returning references
		TGraph m_DataGraph;
	};
}



#endif