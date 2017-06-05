#ifndef _ELECTRONICSELEMENTSPICE_H_
#define _ELECTRONICSELEMENTSPICE_H_

#include <string>
#include <list>
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
		
		virtual void SetDebug(bool set);
		
	private:
		//init all members
		void Init(const std::string & file, std::string & input, std::string & output);
	
		//load spice circuit into ngspice
		void LoadCircuit(const std::string & file);

		//load data into ngspice
		void LoadSignal(unsigned int size, double * spacere, double * datare, double scaling);
		
		//list with forbidden spice commands
		std::list<std::string> m_SpiceCommandForbidden;
		
		//source name for input stream in spice
		std::string m_InputSourceName;
		
		//name of input net
		std::string m_InputNet;
		
		//name of out`put net
		std::string m_OutputNet;
	};
}



#endif