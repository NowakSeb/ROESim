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
string gYAxisRef = "P0";
string gXAxis = "time";
bool gVerbose = true;


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

void ElectronicsElementSpice::ModifyData(Space space, unsigned int size, double * spacere, double * spaceim, double * datare, double * dataim, double scaling)
{

	
}

void ElectronicsElementSpice::Init(const string & file, string & input, string & output)
{
	SetName(file);
	m_ConstSampling = false; //spice takes everything
	//accepts only time space data
	m_SpaceCapability.push_back(Space::TIME);
	
	//init ngspice callback fcts
	int ret = ngSpice_Init(ng_getchar, ng_getchar, ng_exit, ng_data, ng_initdata, ng_thread_runs, NULL);
}

void ElectronicsElementSpice::SetDebug(bool set)
{
	ElectronicsElementBase::SetDebug(set);
	gVerbose = set;
}


void ElectronicsElementSpice::LoadCircuit(const string & file)
{
	string pCommand = "source " + file;
 	ngSpice_Command(const_cast<char*>(pCommand.c_str()));
	ngSpice_Command("circbyline V2 N006 0 PWL(0 1 1 2 1 3");
	ngSpice_Command("circbyline .end");
}

void ElectronicsElementSpice::LoadSignal(unsigned int size, double * spacere, double * datare)
{
// 	for(unsigned int i = 0; i < input.size(); i++)
// 	{
// 		stringstream pInputStream;
// 		if (i==0)
// 		{
// 			pInputStream << "circbyline V2 N006 0 PWL(";
// 		}
// 		else
// 		{
// 			pInputStream << "circbyline +    ";
// 		}
// 		pInputStream << input[i];
// 		if (i == input.size() - 1)
// 		{
// 			pInputStream << ")";
// 		}
// 		ret = ngSpice_Command(const_cast<char *>(pInputStream.str().c_str()));
// 	}
}

//ng spice callback routines
int ng_getchar(char * outputreturn, int ident, void* userdata)
{
	if (gVerbose) {
		cout << outputreturn << endl;
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
	// 	if (gSpiceOutputEnable)
	// 	{
		double pTime = vdata->vecsa[gVecTime]->creal;
		double pVal = vdata->vecsa[gVecOutput]->creal;
		double pRef = vdata->vecsa[gVecOutputRef]->creal;
		unsigned int pPoint = gBufferGraph->GetN();
		// 		cout << pTime << " " << pVal << endl;
		if (gYAxisRef != "")
		{
			gBufferGraph->SetPoint(pPoint, pTime + gSpiceMinTime, pVal-pRef);
		}
		else
		{
			gBufferGraph->SetPoint(pPoint, pTime + gSpiceMinTime, pVal);
		}
		// 	}
		return 0;
}


int ng_initdata(pvecinfoall intdata, int ident, void* userdata)
{
	// Callback function called from bg thread in ngspice once upon intialization of the simulation vectors)
	
	int pCount = intdata->veccount;
	for (int i = 0; i < pCount; i++) {
		//  		printf("Vector: %s\n", intdata->vecs[i]->vecname);
		if (cieq(intdata->vecs[i]->vecname, const_cast<char *>(gYAxis.c_str())))
		{
			gVecOutput = i;
		}
		else if (cieq(intdata->vecs[i]->vecname, const_cast<char *>(gYAxisRef.c_str())))
		{
			gVecOutputRef = i;
		}
		else if (cieq(intdata->vecs[i]->vecname, const_cast<char *>(gXAxis.c_str())))
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
