#ifndef _ELECTRONICSELEMENTAMPLIFIER_H_
#define _ELECTRONICSELEMENTAMPLIFIER_H_

#include <string>
#include <list>
#include <TGraph.h>

#include "ElectronicsElementBase.h"

namespace Electronics
{
	class ElectronicsElementAmplifier : public ElectronicsElementBase
	{
	public:
		
		// Constructor, if bandwidth = 0, no bandwidth is applied
		ElectronicsElementAmplifier(double gain, double bandwidth = 0);
		
		// Destructor
		virtual ~ElectronicsElementAmplifier();
		
		virtual void StoreTransferData(TFile & file);

		virtual void ModifyData(Space space, unsigned int & size, double * & spacere, double * & spaceim, double * & datare, double * & dataim, double scaling);
		
	private:
		
		//amplifier gain
		double m_Gain;

		//amplifier bandwidth
		double m_Bandwidth;
	};
}



#endif