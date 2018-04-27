#ifndef _ELECTRONICSELEMENTBASE_H_
#define _ELECTRONICSELEMENTBASE_H_

#include <fstream>
#include <TFile.h>
#include <TGraph.h>

// #include "ElectronicsElementData.h"

namespace Electronics
{
	enum class Space {TIME, FOURIER}; 
	
	class ElectronicsElementBase
	{
	public:
		
		// Constructor
		ElectronicsElementBase();
		// Destructor
		virtual ~ElectronicsElementBase();
		
// 		//process data
// 		void Process(Electronics::ElectronicsElementData & data);
		
		virtual void StoreTransferData(TFile & file) = 0;
		
		//routine for data modification
		virtual void ModifyData(Space space, unsigned int & size, double * & spacere, double * & spaceim, double * & datare, double * & dataim, double scaling) = 0;
		
		//does this instance needs constant sampling
		bool NeedsConstSampling()
		{
			return m_ConstSampling;
		}
		
		//does this instance accepts fft data
		bool CapableOfSpace(Electronics::Space space);
		
		//returns default space (used if current space is not supported)
		Space DefaultSpace()
		{
			return m_SpaceCapability[0];
		}

		//returns if device provides binary output
		bool BinaryOutput()
		{
			return m_BinaryOutput;
		}
		
		//returns if device provides binary output
		void BinaryOnly(bool set)
		{
			m_BinaryOnly = set;
		}
		
		
		//return vector with leading and trailing edges, if m_BinaryOutput is enabled
		void GetEdges(std::vector<double> & leading, std::vector<double> & trailing);

		virtual void SetDebug(bool set);
//		void AddInputNoise();

//		void AddOutputNoise();
		
	protected:
		
		void SetName(const std::string & name)
		{
			size_t pPos = name.rfind("/");
			m_Name = name.substr(pPos+1, name.size());
		}
		
		//read line from file
		static std::string ReadLine(std::ifstream & file);
		//read line from file, ignoring lines starting with comment
		static std::string NextLine(std::ifstream & file, char comment = '!');
		//instance namespace
		std::string m_Name;
		//does this element needs constantant sampling; true if yes
		bool m_ConstSampling;
		//does element provide binary Output
		bool m_BinaryOutput;
		//list of spaces this element accepts
		std::vector<Electronics::Space> m_SpaceCapability;
		//print debug information to std::out
		bool m_Debug;
		//time of leading edge (if binary output enabled)
		std::vector<double> m_EdgeLeading;
		//time of leading edge (if binary output enabled)
		std::vector<double> m_EdgeTrailing;
		//only binary output enabled (speed up), only useful if binary output is possible
		bool m_BinaryOnly;
		
	};
}



#endif
