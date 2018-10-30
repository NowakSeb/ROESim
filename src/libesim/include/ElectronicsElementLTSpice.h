#ifndef _ELECTRONICSELEMENTLTSPICE_H_
#define _ELECTRONICSELEMENTLTSPICE_H_

#include <string>
#include <list>
#include <TGraph.h>

#include "ElectronicsElementBase.h"

namespace Electronics
{
	class ElectronicsElementLTSpice : public ElectronicsElementBase
	{
	public:
		
		// Constructor
		ElectronicsElementLTSpice(const std::string & path, const std::string & file, const std::string & input, const std::string & output);
		
		// Destructor
		virtual ~ElectronicsElementLTSpice();
		
		virtual void StoreTransferData(TFile & file);

		virtual void ModifyData(Space space, unsigned int & size, double * & spacere, double * & spaceim, double * & datare, double * & dataim, double scaling);
		
		virtual void SetDebug(bool set);
		
	private:
		//name of input spice file
		std::list<std::string> m_SpiceInput;

		//name of input net
		std::string m_LTSpicePath;
		
		//name of input file
		std::string m_InputFile;
		
		//name of out`put net
		std::string m_OutputNet;
		
		//name of intermediate spice file 
		static const std::string m_IntFile;
		
		//name of result spice file 
		static const std::string m_ResFile;
		
		//read fct for LTSpice
		void ReadRawFile(const std::string & filename, double * & time, double * & data, unsigned int & length, double scaling);
	};
}



#endif