#ifndef _ELECTRONICSELEMENTDATA_H_
#define _ELECTRONICSELEMENTDATA_H_

#include <iostream>

#include <TFile.h>
#include <TGraph.h>

#include "ElectronicsElementBase.h"

namespace Electronics
{
	enum class Precision {FULL, HIGHESTPOWER2};
	
	class ElectronicsElementData
	{
	public:
		
		/**
		 * Time space constructor
		 * @param input data in time space
		 * @param ticks time unit in seconds
		 * @param precision calculation precision used
		 * @param sort if true, data is sorted in timespace
		 */
		ElectronicsElementData(const TGraph & input, double ticks = 1, Precision precision = Precision::FULL, bool sort = false);

		/**
		 * Fourier space constructor
		 * @param input0 first data coordinate (complex:true -> real value, otherwise amplitude)
		 * @param input0 second data coordinate  (complex:true -> im value, otherwise phase in rad)
		 * @param complex if true, complex input, otherwise amplitude and phased
		 * @param ticks time unit in seconds
		 * @param precision calculation precision used
		 * @param sort if true data is sorted along space coordinate
		 */
		ElectronicsElementData(const TGraph & input0, const TGraph & input1, bool complex, double ticks = 1, Precision precision = Precision::FULL, bool sort = false);
		
		// Destructor
		virtual ~ElectronicsElementData();
		
		Space GetSpace()
		{
			return m_CurrentSpace;
		}

		Precision GetPrecision()
		{
			return m_Precision;
		}

		void SetPrecision(Precision precision)
		{
			m_Precision = precision;
		}
		
		void SetLog(bool set)
		{
			m_Logging = set;
		}
		
		//return ticks conversion factor
		double GetScaling()
		{
			return m_Scaling;
		}
		
		/**
		 * set space (fourier is true, time is false) 
		 *if fourier sampling is set to const
		 */
		void SetSpace(Electronics::Space set);
		
		/**
		 * set constant sampling with difference between two points in ticks 
		 * @param sampling difference between two points in ticks, if < 0 default default sampling is used
		 */
		void SetSampling(double sampling = -1);
		
		//return current sampling, 0 if not const sampled
		double GetSampling();

		//get data as TGraph
		TGraph & GetDataTGraph();

		//get data as TGraph
		void GetDataTGraph(TGraph & graph1, TGraph & graph2);
		
		//get full data
		void GetDataFull(std::vector<double> & spacere, std::vector<double> & spaceim, std::vector<double> & datare, std::vector<double> & dataim);
		
		//modify the data
		void ApplyElement(ElectronicsElementBase & element);
		
		//send information about signal to std::out
		void PrintDataInfo();
		
	protected:
		
		//common constructor routine
		void Init();
		
		//set data as TGraph
		void SetDataTimeSpace(const TGraph & graph, double ticks = 1, bool sort = false);

		//set data as TGraph
		void SetDataFourierSpace(const TGraph & graph0, const TGraph & graph1, bool complex, double ticks = 1);
		
		/**
		 * update sampling settings
		 * @param updatedata if true then data is sampled with minimal time diff 
		 */
		void UpdateSampling(bool updatedata = false);
		
		//convert the data from time to fourier space
		void ForwardFFT();

		//convert the data from fourier to time space
		void BackwardFFT();

		/**
		 * Change sampling rate in time space
		 * @param sampling new sampling intervall, if -1, the default rate is used
		 * @param input if set to 0, the internal data pointers are used 
		 */
		void ChangeSamplingTimeSpace(double sampling = -1, TGraph * input = 0);
		
		//delete data and space arrys (if set)
		void DeleteData();
		
		inline void Verbose(const char * mes)
		{
			if (m_Logging) {
				std::cout << "ElementData: " << mes << std::endl;
			}
		}
		
		
		//precision for calculations
		Electronics::Precision m_Precision;
		
		//scaling to SI units
		double m_Scaling;
		//is data in time space (false) or fourier space(true)
 		Electronics::Space m_CurrentSpace;
		//sampling rate, if 0 not constant
		double m_SamplingCurrent;
		//is the data const sampled
		bool m_ConstSampled;
// 		//space coordinate minimum value (for const sampling)
// 		double m_ConstSamplingSpaceMin;
		//default sampling (defined by input) -> e.g. time space: time difference between two points
		double m_SamplingDefault;
		//initial offset determined at loading the data, real coordinate
		
		
		unsigned int m_DataSize; //Data size
		double * m_DataRe; //Data coordinate real part
		double * m_DataIm; //Data coordinate imaginary part
		double * m_SpaceRe; //Space coordinate real part
		double * m_SpaceIm; //Space coordinate imaginary part
		
 		TGraph m_GetDataTGraph;
		
		//flag for logging
		bool m_Logging;
		
// 		unsigned int m_DataFourierSize; //Fourier data size
// 		double * m_DataFourierRe; //Fourier space real part
// 		double * m_DataFourierIm; //Fourier space imaginary part
// 		double m_FreqMin; //Fourier space minimum frequency
// 		double m_FreqDelta; //Fourier space frequency delta
	};
}



#endif