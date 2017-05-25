#include <string>
#include <sstream>
#include <vector>
#include <TVirtualFFT.h>
#include <TMath.h>
#include <fftw3.h>

#include "ElectronicsElementData.h"
#include "ElectronicsElementBase.h"

using namespace Electronics;
using namespace std;

inline double GetDelta(unsigned int size, double * data, bool min = true)
{
	double pRes = TMath::Abs(data[1] - data[0]);
	for(unsigned int i = 1; i < size-1; i++) {
		double pDelta = TMath::Abs(data[i+1] - data[i]);
		if (pDelta != pRes) {
// 			cout << i << "res: " << pDelta << pRes << endl;
		}
		if (pDelta < pRes && min) {
			pRes = pDelta;
		}
		else if (pDelta > pRes && !min) {
			pRes = pDelta;
		}
	}
	return pRes;
}

inline double GetDelta(const TGraph & data, bool min = true)
{
	return GetDelta(data.GetN(), data.GetX(), min);
}


ElectronicsElementData::ElectronicsElementData(const TGraph & graph, double ticks, Precision precision, bool sort)
{
	m_CurrentSpace = Space::TIME;
	m_Precision = precision;
	Init();
	
	//copy data and set members accordingly, time space!
	SetDataTimeSpace(graph, ticks);
}

ElectronicsElementData::ElectronicsElementData(const TGraph & input0, const TGraph & input1, bool complex, double ticks, Precision precision, bool sort)
{
	m_CurrentSpace = Space::FOURIER;
	m_Precision = precision;
	Init();

	//copy data and set members accordingly, fourier space!
	SetDataFourierSpace(input0, input1, complex, ticks);
}

void ElectronicsElementData::Init()
{
	m_SamplingCurrent = 0;
	m_SamplingDefault = 0;
	m_Logging = false;
	m_ConstSampled = false;
	
	m_DataRe = 0;
	m_DataIm = 0;
	m_SpaceRe = 0;
	m_SpaceIm = 0;
}



ElectronicsElementData::~ElectronicsElementData()
{
	DeleteData();
	fftw_cleanup();
}

void ElectronicsElementData::DeleteData()
{
	if (m_DataRe != NULL) {
		delete[] m_DataRe;
	}
	if (m_DataIm != NULL) {
		delete[] m_DataIm;
	}
	if (m_SpaceRe != NULL) {
		delete[] m_SpaceRe;
	}
	if (m_SpaceIm != NULL) {
		delete[] m_SpaceIm;
	}
	m_DataRe = NULL;
	m_DataIm = NULL;
	m_SpaceRe = NULL;
	m_SpaceIm = NULL;
}

void ElectronicsElementData::SetSpace(Space set)
{
	if (set == m_CurrentSpace) {
		//current space, no change
		return;
	}
	
	if (m_CurrentSpace == Space::FOURIER && set == Space::TIME) {
		BackwardFFT();
		return;
	}
	if (m_CurrentSpace == Space::TIME && set == Space::FOURIER) {
		
		if (!m_ConstSampled) {
			//const sampling with rate according to precision setting
			double sampling;
			if (m_Precision == Precision::FULL) {
				sampling = GetDelta(m_DataSize, m_SpaceRe, true);
				if (m_SamplingDefault < sampling) {
					sampling = m_SamplingDefault;
				}
			}
			else {
				sampling = m_SamplingDefault;
			}
			ChangeSamplingTimeSpace(sampling);
		}
		ForwardFFT();
		return;
	}
	else {
		//not implemented yet
		throw string("This space conversion is not implemented.");
	}
}

void ElectronicsElementData::SetSampling(double sampling)
{
	if (m_CurrentSpace == Space::TIME){
		double pSampling = sampling;
		
		if (sampling < 0) {
			//const sampling with rate according to precision setting
			if (m_Precision == Precision::FULL) {
				pSampling = GetDelta(m_DataSize, m_SpaceRe, true);
				if (m_SamplingDefault < pSampling) {
					pSampling = m_SamplingDefault;
				}
			}
			else {
				pSampling = m_SamplingDefault;
			}
		}
		ChangeSamplingTimeSpace(pSampling);
		Verbose("Sampling changed.");
	}
	else if (m_CurrentSpace == Space::FOURIER) {
		throw string("No sampling conversion implemented for FFT yet.");
	}
	else {
		throw string("No sampling conversion implemented for this space.");
	}
}

double ElectronicsElementData::GetSampling()
{
	if (m_ConstSampled) {
		return m_SamplingCurrent;
	}
	else {
		return 0;
	}
}


TGraph & ElectronicsElementData::GetDataTGraph()
{
	if (m_CurrentSpace == Space::TIME) {
		m_GetDataTGraph = TGraph(m_DataSize, m_SpaceRe, m_DataRe);
	}
	else {
		throw string("This space cannot be converted in a TGraph.");
	}
	
	return m_GetDataTGraph;
}

void ElectronicsElementData::GetDataTGraph(TGraph & graph1, TGraph & graph2)
{
	if (m_CurrentSpace == Space::TIME) {
		graph1 = TGraph(m_DataSize, m_SpaceRe, m_DataRe);
		graph2 = TGraph();
	}
	else if (m_CurrentSpace == Space::FOURIER) {
		graph1 = TGraph(m_DataSize, m_SpaceRe, m_DataRe);
		graph2 = TGraph(m_DataSize, m_SpaceRe, m_DataIm);
	}
	else {
		throw string("This space cannot be converted in two TGraph.");
	}
}


void ElectronicsElementData::GetDataFull(std::vector<double> & spacere, std::vector<double> & spaceim, std::vector<double> & datare, std::vector<double> & dataim)
{
	spacere.clear();
	spaceim.clear();
	datare.clear();
	dataim.clear();
	for(unsigned int i = 0; i < m_DataSize; i++) {
		spacere.push_back(m_SpaceRe[i]);
		spaceim.push_back(m_SpaceIm[i]);
		datare.push_back(m_DataRe[i]);
		dataim.push_back(m_DataIm[i]);
	}
}


void ElectronicsElementData::SetDataTimeSpace(const TGraph & graph, double ticks, bool sort)
{
	double pOffset;
	
	if (graph.GetN() < 2) {
		stringstream pStream;
		pStream << "Input TGraph has " << graph.GetN() << " data points. How should this work out?";
		throw pStream.str();
	}
	m_Scaling = ticks;
	m_CurrentSpace = Space::TIME;
	//delete old data
	DeleteData();
	//fill new data
	m_SpaceRe = new double[graph.GetN()];
	m_DataRe = new double[graph.GetN()];
	m_DataSize = graph.GetN();
	if (sort) {
		TGraph pGraph(graph);
		pGraph.Sort();
		pGraph.GetPoint(0, pOffset, m_DataRe[0]);
		for(int i = 0; i < pGraph.GetN(); i++) {
			pGraph.GetPoint(i, m_SpaceRe[i], m_DataRe[i]);
			m_SpaceRe[i] -= pOffset;
		}
	}
	else {
		graph.GetPoint(0, pOffset, m_DataRe[0]);
		for(int i = 0; i < graph.GetN(); i++) {
			graph.GetPoint(i, m_SpaceRe[i], m_DataRe[i]);
			m_SpaceRe[i] -= pOffset;
		}
	}
	UpdateSampling(true);
}

void ElectronicsElementData::SetDataFourierSpace(const TGraph & graph0, const TGraph & graph1, bool complex, double ticks)
{
	if (graph0.GetN() < 2 || graph1.GetN() < 2) {
		stringstream pStream;
		pStream << "Input TGraphs have " << graph0.GetN() << " and " << graph0.GetN() << " data points. How should this work out?";
		throw pStream.str();
	}
	m_Scaling = ticks;
	m_CurrentSpace = Space::FOURIER;
	//check for min sampling -> fourier needs to be fixed sampled
	//furthermore, fourier space must start with frequency -> no negative freqencies!
	double pMinDelta = GetDelta(graph0, true);
	double pMinDelta1 = GetDelta(graph1, true);
	if (pMinDelta1 < pMinDelta) {
		pMinDelta = pMinDelta1;
	}
	double pMinX, pMaxX, pMinY, pMaxY;
	double pMaxX1;
	graph0.ComputeRange(pMinX, pMinY, pMaxX, pMaxY);
	graph0.ComputeRange(pMinX, pMinY, pMaxX1, pMaxY);
	if (pMaxX1 > pMaxX) {
		pMaxX = pMaxX1;
	}
	//just care about max, min is set to 0 -> negative values are automatically ignored
	m_DataSize = pMaxX / pMinDelta + 1;
	DeleteData();
	//double size for fft lib
	m_SpaceRe = new double[m_DataSize*2];
	m_DataRe = new double[m_DataSize*2];
	m_DataIm = new double[m_DataSize*2];
	
	if (complex) {
		for(unsigned int i = 0; i < m_DataSize; i++) {
			m_SpaceRe[i] = pMinDelta * i;
			m_DataRe[i] = graph0.Eval(m_SpaceRe[i]);
			m_DataIm[i] = graph1.Eval(m_SpaceRe[i]);
		}
	}
	else {
		for(unsigned int i = 0; i < m_DataSize; i++) {
			m_SpaceRe[i] = pMinDelta * i;
			double pRad = graph0.Eval(m_SpaceRe[i]);
			double pPhase = graph1.Eval(m_SpaceRe[i]);
			m_DataRe[i] = pRad * TMath::Cos(pPhase);
			m_DataIm[i] = pRad * TMath::Sin(pPhase);
		}
	}
	
	m_ConstSampled = true;
	m_SamplingDefault = TMath::Abs(pMinDelta);
	m_SamplingCurrent = TMath::Abs(pMinDelta);
}

void ElectronicsElementData::ApplyElement(ElectronicsElementBase & element)
{
	if (!(element.CapableOfSpace(m_CurrentSpace))) {
		SetSpace(element.DefaultSpace());
	}
	
	//and apply
	element.ModifyData(m_CurrentSpace, m_DataSize, m_SpaceRe, m_SpaceIm, m_DataRe, m_DataIm, m_Scaling);
}

void ElectronicsElementData::UpdateSampling(bool updatedata)
{
	if (m_CurrentSpace == Space::TIME) {
		
		//init members
		m_ConstSampled = true;
		m_SamplingDefault = 0;
		m_SamplingCurrent = 0;
		//loop over data
		double pMinTimeDiff = GetDelta(m_DataSize, m_SpaceRe, true);
		double pMaxTimeDiff = GetDelta(m_DataSize, m_SpaceRe, false);
		
		if (pMaxTimeDiff == pMinTimeDiff) {
			m_ConstSampled = true;
			m_SamplingDefault = TMath::Abs(pMaxTimeDiff);
			m_SamplingCurrent = TMath::Abs(m_SamplingDefault);
		}
		else {
			m_ConstSampled = false;
		}
		if (updatedata) {
			m_SamplingDefault = TMath::Abs(pMinTimeDiff);
			ChangeSamplingTimeSpace(pMinTimeDiff);
		}
	}
	else {
		throw string("UpdateSampling is not defined for this space yet.");
	}
}

void ElectronicsElementData::ForwardFFT()
{

	fftw_complex *pOut;
	fftw_plan pFFTPlan;
	pOut = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * m_DataSize);
	pFFTPlan = fftw_plan_dft_r2c_1d(m_DataSize, m_DataRe, pOut, FFTW_ESTIMATE);

	fftw_execute(pFFTPlan);

// 	From above, an FFTW_FORWARD transform corresponds to a sign of -1 in the exponent of the DFT. Note also that we use the standard “in-order” output ordering—the k-th output corresponds to the frequency k/n (or k/T, where T is your total sampling period). 

	//zero element is dc
	//n/2th is nyqusit -> m_SamplingCurrent
	
// 	double pTimeBegin = m_SpaceRe[0];
// 	double pTimeEnd = m_SpaceRe[m_DataSize-1];
// 	double pTimeLength = pTimeEnd - pTimeBegin;
// 	m_SamplingCurrent = 1./ (pTimeLength / (m_DataSize-1));
// 	m_SamplingCurrent *= 2; //nyquist

	//convert sampling to fourier space
	m_SamplingCurrent = 1. / m_SamplingCurrent; //max freq
	m_SamplingCurrent = m_SamplingCurrent / m_DataSize;

	//set new size according to fftw3 manual (half + dc)
	m_DataSize = (int) m_DataSize/2;
	m_DataSize++;
	
	DeleteData();
	m_SpaceRe = new double [m_DataSize*2];
	m_DataRe = new double [m_DataSize*2];
	m_DataIm = new double[m_DataSize*2];
	
	for(unsigned int i = 0; i < m_DataSize*2; i++) {
		m_SpaceRe[i] = i * m_SamplingCurrent;
		m_DataRe[i] = pOut[i][0];
		m_DataIm[i] = pOut[i][1];
	}
	m_CurrentSpace = Space::FOURIER;
	m_Scaling = 1./ m_Scaling;
	
	//clean up
	fftw_destroy_plan(pFFTPlan);
	fftw_free(pOut);
}

void ElectronicsElementData::BackwardFFT()
{
	//output ordering—the k-th output corresponds to the frequency k/n (or k/T, where T is your total sampling period).(The frequency -k/n is the same as the frequency (n-k)/n.) 

	fftw_complex *pIn;
	fftw_plan pFFTPlan;
	
	pIn = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * m_DataSize * 2);

	//prepare data 
	for(unsigned int i = 0; i < m_DataSize; i++) {
		pIn[i][0] = m_DataRe[i];
		pIn[i][1] = m_DataIm[i];
		pIn[i+m_DataSize][0] = 0;
		pIn[i+m_DataSize][1] = 0;
	}
	//set new size (see FFTW3 manual and FowardFFT)
	m_DataSize--;
	m_DataSize *= 2;
	
	//allocate new space
	DeleteData();
	m_SpaceRe = new double [m_DataSize];
	m_DataRe = new double [m_DataSize];
	//and transform
	pFFTPlan = fftw_plan_dft_c2r_1d(m_DataSize, pIn, m_DataRe, FFTW_ESTIMATE);
	fftw_execute(pFFTPlan);

	//set sampling to time space
	m_SamplingCurrent = m_SamplingCurrent * m_DataSize;
	m_SamplingCurrent = 1. / m_SamplingCurrent;
	
	//scale data, see FFTW3 manual
	//calculate time
	for(unsigned int i = 0; i < m_DataSize; i++) {
		m_DataRe[i] /= m_DataSize;
		m_SpaceRe[i] = i * m_SamplingCurrent;
	}
	
	m_Scaling = 1./ m_Scaling;
	m_CurrentSpace = Space::TIME;

	//clean up
	fftw_free(pIn);
}

void ElectronicsElementData::ChangeSamplingTimeSpace(double sampling, TGraph * input)
{
	TGraph * pGraph;

	if (sampling < 0) {
		//does not make any sense
		m_SamplingCurrent = m_SamplingDefault;
		Verbose("Sampling is lower 0. Last valid value is used.");
	}
	else {
		m_SamplingCurrent = sampling;
	}

	double pMax;
	
	//fill tgraph and determine maximum time
	if (input == NULL)
	{
		pGraph = new TGraph(m_DataSize, m_SpaceRe, m_DataRe);
		pMax = m_SpaceRe[m_DataSize - 1];
	}
	else {
		double y;
		pGraph = input;
		pGraph->GetPoint(pGraph->GetN() - 1, pMax, y);
	}
	unsigned int pLength = pMax / m_SamplingCurrent + 1;
	double * pX = new double[pLength];
	double * pY = new double[pLength];
	
	for(unsigned int i = 0; i < pLength; i++) {
		pX[i] = i*sampling;
		pY[i] = pGraph->Eval(pX[i]);
	}
	//delete arrays
	DeleteData();
	if (input == NULL) {
		delete pGraph;
	}
	
	m_SpaceRe = pX;
	m_DataRe = pY;
	m_DataSize = pLength;
	m_ConstSampled = true;
	m_SamplingCurrent = sampling;
}

void ElectronicsElementData::PrintDataInfo()
{
	if (m_CurrentSpace == Space::TIME) {
		cout << "Space: Time" << endl;
	}
	if (m_CurrentSpace == Space::FOURIER) {
		cout << "Space: Fourier" << endl;
	}
	else {
		cout << "Space: Unknown" << endl;
	}
	
	cout << "Const sampling: " << m_ConstSampled << endl;
	cout << "Sampling rate: " << m_SamplingCurrent << endl;
	cout << "Data points: " << m_DataSize << endl;
	cout << "Data points: " << m_Scaling << endl;
	
	if (m_CurrentSpace == Space::TIME) {
		cout << "todo...";
	}
	if (m_CurrentSpace == Space::FOURIER) {
		double pMaxSignal = 0;
		double pMaxFreq = 0;
		
		for(unsigned int i = 0; i < m_DataSize; i++) {
			double pVal = TMath::Sqrt(m_DataRe[i] * m_DataRe[i] + m_DataIm[i] * m_DataIm[i]);
			if (pVal > pMaxSignal) {
				pMaxSignal = pVal;
				pMaxFreq = m_SpaceRe[i];
			}
		}
		cout << "Amplitude max: " << pMaxSignal << " @ " << pMaxFreq << endl;
	}
	else {
		throw string("PrintDataInfo is not defined for this space yet.");
	}
}
