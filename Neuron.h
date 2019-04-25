//---------------------------------------------------------------------------
#ifndef NeuronH
#define NeuronH
#include <vector>
#include "stdio.h"
#include "math.h"
//---------------------------------------------------------------------------
typedef unsigned int _TopologyID;
//---------------------------------------------------------------------------
class _NParameters {
	private:
        unsigned int t;
		unsigned int Xm, Ym;
		double k1, k2, k3;
		double Lbd;
		double D;
	public:
		_NParameters();
		_NParameters(_NParameters*);
		_NParameters(unsigned int, unsigned int, double, double, double);
		double pFunction(double, double);
		double GammaFunction(double, double, unsigned int, int);
		double FiFunction(double, double, double, double);
		std::vector<double> CompareCPFunction(std::vector< std::pair<double, double> >, std::vector< std::pair<double, double> >);
		double CompareCPFunctionD(std::vector< std::pair<double, double> >, std::vector< std::pair<double, double> >);
		double CompareFunction(double, double, double, double);
        double GetFiVectorLength(double);
		std::pair<double, double> GetFiVector(double, double, double, double, double);
        void TimeZero();
		void TimeForward();
		void SetArea(unsigned int, unsigned int);
		void Setk1(double);
		void Setk2(double);
		void Setk3(double);
		void SetLbd(double);
		unsigned int GetTime();
		std::pair<unsigned int, unsigned int> GetArea();
		double Getk1();
		double Getk2();
		double Getk3();
		double GetLbd();
		~_NParameters();
};

class _NReceptor
{
	private:
		_NParameters *Parameters;
		std::vector< std::pair<double, double> > CP, CPf;
		double x, y;
		double x0, y0;
		double xf, yf;
		double rc;
		double l, lf;
		bool Locked;
	public:
		_NReceptor();
		_NReceptor(_NReceptor*, _NParameters*);
		_NReceptor(_NParameters*, double, double);
		bool CheckActive(double);
		void DoLock();
		void DoUnlock();
		bool GetLocked();
		void SetPos(double, double);
		void SavePos();
		void SetRc(double);
		std::pair<double, double> GetPos();
		std::pair<double, double> GetPos0();
		std::pair<double, double> GetPosf();
		std::vector< std::pair<double, double> > GetCP();
		std::vector< std::pair<double, double> > GetCPf();
		double Getl();
		double Getlf();
		double GetRc();
		~_NReceptor();
};

class _NSynaps
{
	private:
		_NParameters *Parameters;
		double x, y;
		unsigned int Tl;
		int Type;
		double p;
		double Gamma, dGamma;
	public:
		_NSynaps();
		_NSynaps(_NSynaps*, _NParameters*);
		_NSynaps(_NParameters*, double, double, unsigned int, int);
		void In(double);
		void ClearGamma();
		unsigned int GetTl();
		int GetType();
		double Getp();
		double GetGamma();
		double GetdGamma();
		std::pair<double, double> GetPos();
		~_NSynaps();
};

class _NEntry
{
	private:
		_NParameters *Parameters;
		std::vector<_NSynaps*> Synapses;
		std::vector<double> Signal;
        bool *X;
	public:
		_NEntry();
		_NEntry(_NEntry*, _NParameters*);
		_NEntry(_NParameters*);
		void AddSynaps(double, double, unsigned int, int);
		void In(double);
		void ClearSignal();
		_NSynaps* GetSynaps(unsigned int);
        unsigned int GetSynapsesCount();
		~_NEntry();
};

class Neuron
{
	private:
		std::vector<_NEntry*> Entrys;
		std::vector<_NReceptor*> Receptors;
		float P;
		std::pair<double, double> GetRSdPos(_NReceptor*, _NSynaps*);
	public:
		_NParameters *Parameters;
		double FiOnR;
		double RcOnR;
		Neuron();
		Neuron(Neuron*);
		Neuron(unsigned int, unsigned int, double, double, double);
		void CreateNewEntry();
		void CreateNewSynaps(unsigned int, double, double, unsigned int); 				// entry id, x, y, sinaps time latency
		void CreateNewSynaps(unsigned int, double, double, unsigned int, int);          // entry id, x, y, sinaps time latency, neurotransmitter type
		void CreateNewSynaps(unsigned int, double, double, unsigned int, _TopologyID); 	// entry id, x, y, sinaps time latency, receptor claster topology id
		void CreateNewReceptor(double, double);                           				// x, y
		void CreateNewReceptorClaster(double, double, double, _TopologyID);             // x, y, R, receptor claster topology id
		float SignalsSend(double, ...);               				  	  				// signal element
		bool SignalReceive();
		void CreateCheckpoint();
		void ReInit();
		std::vector<double> Neuron::CompareCP();
		double CompareSignals();
		double CompareSignals(bool);
		std::vector<_NEntry*> GetEntrys();
		std::vector<_NReceptor*> GetReceptors();
		unsigned int GetEntrysCount();
		unsigned int GetSynapsesCount();
		unsigned int GetReceptorsCount();
		~Neuron();
};
//---------------------------------------------------------------------------
#endif

