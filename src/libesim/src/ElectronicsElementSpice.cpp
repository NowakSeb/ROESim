#include <string>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <vector>
#include <fstream>

#include <TMath.h>
#include "sharedspice.h"

#include "ElectronicsElementSpice.h"

using namespace Electronics;
using namespace std;

//ngspice callback fcts
int ng_getchar(char* outputreturn, int ident, void* userdata);
int ng_getstat(char* outputreturn, int ident, void* userdata);
int ng_exit(int, bool, bool, int ident, void*);
int ng_thread_runs(bool noruns, int ident, void* userdata);
int ng_initdata(pvecinfoall intdata, int ident, void* userdata);
int ng_data(pvecvaluesall vdata, int numvecs, int ident, void* userdata);
int cieq(register char *p, register char *s);

//ngspice variables
bool gSpiceRunning = true;
static bool errorflag = false;
bool gSpiceOutputEnable;
unsigned int gVecOutput;
unsigned int gVecOutputRef;
unsigned int gVecTime;
TGraph * gBufferGraph;
double gSpiceMinTime = 0;
string gYAxis = "n005";
string gYAxisRef;// = "P0";
bool gVerbose = false;
bool gInit = false;

ElectronicsElementSpice::ElectronicsElementSpice(const std::string & file, string & input, string & output)
{
	Init(file, input, output);
	LoadCircuit(file);
}

ElectronicsElementSpice::~ElectronicsElementSpice()
{
	
}

void ElectronicsElementSpice::StoreTransferData(TFile & file)
{
	
}

// void ElectronicsElementSpice::ModifyData(Space space, unsigned int size, double * spacere, double * spaceim, double * datare, double * dataim, double scaling)
void ElectronicsElementSpice::ModifyData(Space space, unsigned int & size, double * & spacere, double * & spaceim, double * & datare, double * & dataim, double scaling)
{
	list<string>::iterator pIt;
	int pRet;
	stringstream pSimStream, pNameStream;
	double pMaxSpace = spacere[1] - spacere[0];
	
	//.tran tstep tstop <tstart <tmax >> <uic>
	pSimStream << "circbyline .TRAN " << pMaxSpace << " " << spacere[size - 1];  

	pNameStream << "circbyline * ROESIM_" << &size;
	pRet = ngSpice_Command(const_cast<char *>(pNameStream.str().c_str()));
	
	//apply prepared data
	for(pIt = m_SpiceInput.begin(); pIt != m_SpiceInput.end(); pIt++) {
		pRet = ngSpice_Command(const_cast<char *>(pIt->c_str()));
		cout << *pIt << endl;
	}
	
	//now add sources and input data
	for(unsigned int i = 0; i < size; i++)
	{
		stringstream pInputStream;
		if (i==0)
		{
			pInputStream << "circbyline " << m_InputSourceName << " " << m_InputNet;
			pInputStream << " 0 PWL ( ";
// 			"V2 N006 0 PWL(";
// 			pSourceString = "circbyline " + m_InputSourceName + " " + m_InputNet + " 0 pwl (0 0 1 0)";
		}
		else
		{
			pInputStream << "circbyline + ";
		}
		pInputStream << scaling * spacere[i] << " " << datare[i];
		if (i == size - 1)
		{
			pInputStream << " )";
		}
		pRet = ngSpice_Command(const_cast<char *>(pInputStream.str().c_str()));
	}
	
	//other options and simulation type
// 	ngSpice_Command("circbyline .option noinit acct");
	pRet = ngSpice_Command(const_cast<char *>(pSimStream.str().c_str()));
	pRet = ngSpice_Command("circbyline .end");
	
	gYAxis = m_OutputNet;
	gBufferGraph = new TGraph();
	ngSpice_Command("bg_run");
		
	//wait until finished
	for (;;) {
		usleep (10000);
		if (gSpiceRunning)
			break;
	}
	//copy buffer graph
	
	delete spacere;
	delete datare;
	
	size = gBufferGraph->GetN();
	
	//gBufferGraph->Write(pNameStream.str().c_str());
	spacere = new double[size];
	datare = new double[size];
	
	for(unsigned int i = 0; i < size; i++) {
		gBufferGraph->GetPoint(i, spacere[i], datare[i]);
		spacere[i] /= scaling;
	}

	pRet = ngSpice_Command("destroy all");
	pRet = ngSpice_Command("remcirc");
	pRet = ngSpice_Command("reset");
	
	delete gBufferGraph;
}

void ElectronicsElementSpice::Init(const string & file, string & input, string & output)
{
	SetName(file);
	//spice takes also non const sampling everything
	m_ConstSampling = false;
	//accepts only time space data
	m_SpaceCapability.push_back(Space::TIME);
	//input, output
	m_InputNet = input;
	m_OutputNet = output;

	//init ngspice callback fcts
	if (!gInit) {
		//avoid this function being called more tham once
		int ret = ngSpice_Init(ng_getchar, ng_getchar, ng_exit, ng_data, ng_initdata, ng_thread_runs, NULL);
		gInit = true;
	}
	
	stringstream pStream;
// 	pStream << "v" << hex << this;
// 	m_InputSourceName = pStream.str();
	m_InputSourceName = "vin";
	m_SpiceCommandForbidden.clear();
	m_SpiceCommandForbidden.push_back(".trans");
	m_SpiceCommandForbidden.push_back(".ac");
}

void ElectronicsElementSpice::SetDebug(bool set)
{
	ElectronicsElementBase::SetDebug(set);
	gVerbose = set;
}


void ElectronicsElementSpice::LoadCircuit(const string & file)
{
	ifstream pFile;

	pFile.open(file);
	
	if (!pFile.good()) {
		stringstream pStream;
		pStream << "Spice file " << file << " cannot be opened. Does this file exists?";
		throw pStream.str();
	}

	while(pFile.good()) {
		string pSpiceCommand = NextLine(pFile);
		for(list<string>::iterator pIt = m_SpiceCommandForbidden.begin(); pIt != m_SpiceCommandForbidden.end(); pIt++) {
			if (pSpiceCommand.find(*pIt) != pSpiceCommand.npos) {
				string pEx = "Spice file " + file + " contains command " + *pIt + " which is not allowed in this application.";
				throw pEx;
			}
		}
		if (pSpiceCommand.find(".end") != string::npos || pSpiceCommand == "") {
			//sources and end will be defined later
			break;
		}
		else {
			pSpiceCommand = "circbyline " + pSpiceCommand;
			ngSpice_Command(const_cast<char*>(pSpiceCommand.c_str()));
		}
	}
}

// void ElectronicsElementSpice::LoadSignal(unsigned int size, double * spacere, double * datare, double scaling)
// {
// 	stringstream pInputStream;
// 	pInputStream << "let inpvec=vector(" << 2*size << ")";
// 	ngSpice_Command(const_cast<char *>(pInputStream.str().c_str()));
// 	
// 	for(unsigned int i = 0; i < size; i++)
// 	{
// 		stringstream pInputStream1, pInputStream2;
// 		pInputStream1 << "set inpvec[" << 2*i << "]=" << spacere[i]*scaling;
// 		ngSpice_Command(const_cast<char *>(pInputStream1.str().c_str()));
// 		pInputStream2 << "set inpvec[" << 2*i+1 << "]=" << datare[i];
// 		ngSpice_Command(const_cast<char *>(pInputStream2.str().c_str()));
// 	}
// 	string pCommand = "alter @" + m_InputSourceName + "[pwl]=inpvec";
// // 	cout << pCommand << endl;
// 	ngSpice_Command(const_cast<char *>(pCommand.c_str()));
	
	
// 	ngSpice_Command("set");
	//ngSpice_Command("alter vin pwl = [ 0 1 2 3 ]");
	//ngSpice_Command("setcirc");
	//ngSpice_Command("circbyline .control");
// 	ngSpice_Command("alter @vin[pwl]=inpvec");
	//ngSpice_Command("circbyline .endc");
// 	pCommand = "show inpvec";// + m_InputSourceName;
// 	ngSpice_Command(const_cast<char *>(pCommand.c_str()));
	
	// 	ngSpice_Command(const_cast<char *>(pInputStream.str().c_str()));
	
	//ngSpice_Command("display");
// 
// 	for(unsigned int i = 0; i < size; i++)
// 	{
// 		stringstream pInputStream;
// 		if (i==0)
// 		{
// 			pInputStream << "alter " << m_InputSourceName << " pwl = [ ";
// 		}
// 		else
// 		{
// 			pInputStream << "";
// 		}
// 		pInputStream << spacere[i] << " " << datare[i];
// 		if (i == size - 1)
// 		{
// 			pInputStream << " ]";
// 		}
// 		ngSpice_Command(const_cast<char *>(pInputStream.str().c_str()));
// 	}
// 	stringstream pInputStream;
// 	ngSpice_Command("alter @v1[pwl] = [ 0 1 2 3 ]");
// 	ngSpice_Command("alter R1 = 10");
// }

//ng spice callback routines
int ng_getchar(char * outputreturn, int ident, void* userdata)
{
	if (gVerbose) {
		cout << "SPICE:\t" << outputreturn << endl;
	}
	return 0;
}

int ng_thread_runs(bool noruns, int ident, void* userdata)
{
	//callback fct for "spice is running"
	gSpiceRunning = noruns;
	return 0;
}

// Callback function called from bg thread in ngspice once per accepted data point
int ng_data(pvecvaluesall vdata, int numvecs, int ident, void* userdata)
{
	double pTime = vdata->vecsa[gVecTime]->creal;
	double pVal = vdata->vecsa[gVecOutput]->creal;
	double pRef = vdata->vecsa[gVecOutputRef]->creal;
	unsigned int pPoint = gBufferGraph->GetN();
	if (gYAxisRef != "") {
		gBufferGraph->SetPoint(pPoint, pTime + gSpiceMinTime, pVal-pRef);
	}
	else {
		gBufferGraph->SetPoint(pPoint, pTime + gSpiceMinTime, pVal);
	}
	return 0;
}


int ng_initdata(pvecinfoall intdata, int ident, void* userdata)
{
	// Callback function called from bg thread in ngspice once upon intialization of the simulation vectors)
	int pCount = intdata->veccount;
	for (int i = 0; i < pCount; i++) {
		//cout << "ng_init " << intdata->vecs[i]->vecname << endl;
		if (cieq(intdata->vecs[i]->vecname, const_cast<char *>(gYAxis.c_str())))
		{
			gVecOutput = i;
		}
		else if (cieq(intdata->vecs[i]->vecname, const_cast<char *>(gYAxisRef.c_str())))
		{
			gVecOutputRef = i;
		}
		else if (cieq(intdata->vecs[i]->vecname, "time"))
		{
			gVecTime = i;
		}
	}
	return 0;
}
int ng_exit(int exitstatus, bool immediate, bool quitexit, int ident, void * userdata)
{
	// Callback function called from bg thread in ngspice if fcn controlled_exit()
	//   is hit. Do not exit, but unload ngspice.
	
	if(quitexit) {
		cout << "DNote: Returned form quit with exit status " << exitstatus << endl;
		exit(exitstatus);
	}
	if(immediate) {
		cout << "DNote: Unloading ngspice inmmediately is not possible" << endl;
		cout << "DNote: Can we recover?" << endl;
	}
	else {
		cout << "DNote: Unloading ngspice inmmediately is not possible" << endl;
		cout << "DNote: Can we recover? Send 'quit' command to ngspice." << endl;
		errorflag = true;
		ngSpice_Command("quit 5");
	}
	
	return exitstatus;
}

/* Funcion called from main thread upon receiving signal SIGTERM */
void
alterp(int sig) {
	ngSpice_Command("bg_halt");
}


/* Case insensitive str eq. */
/* Like strcasecmp( ) XXX */

int
cieq(register char *p, register char *s)
{
	while (*p) {
		if ((isupper(*p) ? tolower(*p) : *p) !=
			(isupper(*s) ? tolower(*s) : *s))
			return(false);
		p++;
		s++;
	}
	return (*s ? false : true);
}
