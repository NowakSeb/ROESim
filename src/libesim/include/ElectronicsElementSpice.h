#ifndef _ELECTRONICSELEMENTSPICE_H_
#define _ELECTRONICSELEMENTSPICE_H_

#include <string>
#include <vector>
#include <TGraph.h>

#include "ElectronicsElementBase.h"

namespace Electronics
{
	class ElectronicsElementSpice : public ElectronicsElementBase
	{
	public:
		
		// Constructor
		ElectronicsElementSpice(const std::string & file, std::string & input, std::string & output);
		
		// Destructor
		virtual ~ElectronicsElementSpice();
		
		virtual void StoreTransferData(TFile & file);

		virtual void ModifyData(Space space, unsigned int size, double * spacere, double * spaceim, double * datare, double * dataim, double scaling);
		
	private:
		//init all members
		void Init(const std::string & file, std::string & input, std::string & output);
		
	};
}



#endif