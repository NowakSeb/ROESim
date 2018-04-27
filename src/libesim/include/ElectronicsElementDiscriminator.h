#ifndef _ELECTRONICSELEMENTDISCRIMINATOR_H_
#define _ELECTRONICSELEMENTDISCRIMINATOR_H_

#include <string>
#include <list>
#include <TGraph.h>

#include "ElectronicsElementBase.h"

namespace Electronics
{
	class ElectronicsElementDiscriminator : public ElectronicsElementBase
	{
	public:
		
		// Constructor
		ElectronicsElementDiscriminator(double threshold, double low, double high, double hysteresis, double deadtime = 0, double risetime = 0);
		
		// Destructor
		virtual ~ElectronicsElementDiscriminator();
		
		virtual void StoreTransferData(TFile & file);

		virtual void ModifyData(Space space, unsigned int & size, double * & spacere, double * & spaceim, double * & datare, double * & dataim, double scaling);
		
	private:
		
		//discriminator threshold
		double m_Threshold;

		//output level low
		double m_LevelLow;

		//output level high
		double m_LevelHigh;
		
		//hysteresis setting
		double m_Hysteresis;
		
		//deadtime setting
		double m_DeadTime;
		
		//risetime setting
		double m_RiseTime;
	};
}



#endif