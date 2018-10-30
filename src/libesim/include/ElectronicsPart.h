#ifndef _ELECTRONICSPART_H_
#define _ELECTRONICSPART_H_

#include <vector>
#include <list>

//#include "ElectronicsElementBase.h"
#include "ElectronicsElementData.h"
#include "ElectronicsElementSPar.h"
#include "ElectronicsElementSpice.h"
#include "ElectronicsElementLTSpice.h"
#include "ElectronicsElementDiscriminator.h"
#include "ElectronicsElementAmplifier.h"

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
		 * add spice circuit
		 * @param file filename 
		 * @param input name of input net
		 * @param output name of output net
		 */
		void AddElementSpice(const std::string & file, const std::string & input, const std::string & output)
		{
			AddElement(new ElectronicsElementSpice(file, input, output));
		}

		/**
		 * add spice circuit
		 * @param path path to LTSpice (including wine) 
		 * @param file filename, has to include .tran command with correct time length
		 * @param input name of input PWL file
		 * @param output name of output net
		 */
		void AddElementLTSpice(const std::string & path, const std::string & file, const std::string & input, const std::string & output)
		{
			AddElement(new ElectronicsElementLTSpice(path, file, input, output));
		}
		
		/**
		 * add discriminator
		 * @param threshold threshold voltage
		 * @param low output level low
		 * @param high output level high
		 * @param hysteresis hysteresis setting
		 * @param deadtime minimum difference between two trailing edges
		 */
		void AddElementDiscriminator(double threshold, double low = 0, double high = 1, double hysteresis = 0, double deadtime = 0, double risetime = 0)
		{
			AddElement(new ElectronicsElementDiscriminator(threshold, low, high, hysteresis, deadtime, risetime));
		}

		/**
		 * add amplifier
		 * @param gain amplifier gain
		 * @param bandwidth amplifier bandwidth
		 */
		void AddElementAmplifier(double gain, double bandwidth = 0)
		{
			AddElement(new ElectronicsElementAmplifier(gain, bandwidth));
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
		
		void SetDebug(bool set = true);
		
		/**
		 * returns if binary output (see GetLatestEdges) is available
		 */
		bool BinaryOutput();
		
		/**
		 * Get list of edges if last elemnt is binary based
		 * @param leading leading edges
		 * @param trailing trailing edges
		 */
		void GetLatestEdges(std::vector<double> & leading, std::vector<double> & trailing);
		
	protected:
		
		void AddElement(ElectronicsElementBase * element);

		//list of all elements
		std::list<ElectronicsElementBase *> m_Elements;
		
		//precision used for calculation
		Electronics::Precision m_Precision;
		
		//instance used for returning references
		TGraph m_DataGraph;
		
		//is debug mode enabled
		bool m_Debug;
	};
}



#endif