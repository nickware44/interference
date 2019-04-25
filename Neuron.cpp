//---------------------------------------------------------------------------
#pragma hdrstop
#pragma package(smart_init)
#include "Neuron.h"
//---------------------------------------------------------------------------
Neuron::Neuron()
{
	P = 0;
	Parameters = new _NParameters();
}

Neuron::Neuron(Neuron *N) {
	Parameters = new _NParameters(N->Parameters);
	for (int i = 0; i < N->GetEntrys().size(); i++) {
		_NEntry *E = new _NEntry(N->GetEntrys()[i], Parameters);
		Entrys.push_back(E);
	}
	for (int i = 0; i < N->GetReceptors().size(); i++) {
		_NReceptor *R = new _NReceptor(N->GetReceptors()[i], Parameters);
		Receptors.push_back(R);
	}
}

Neuron::Neuron(unsigned int Xm, unsigned int Ym, double k1, double k2, double Lbd)
{
	P = 0;
	Parameters = new _NParameters(Xm, Ym, k1, k2, Lbd);
}

std::pair<double, double> Neuron::GetRSdPos(_NReceptor*, _NSynaps*)
{

}

void Neuron::CreateNewEntry()
{
	_NEntry *E = new _NEntry(Parameters);
    Entrys.push_back(E);
}

void Neuron::CreateNewSynaps(unsigned int EID, double x, double y, unsigned int Tl)
{
	Entrys[EID] -> AddSynaps(x, y, Tl, 0);
}

void Neuron::CreateNewSynaps(unsigned int EID, double x, double y, unsigned int Tl, int Type)
{
	Entrys[EID] -> AddSynaps(x, y, Tl, Type);
}

void Neuron::CreateNewReceptor(double x, double y)
{
	_NReceptor *R = new _NReceptor(Parameters, x, y);
	Receptors.push_back(R);
}

void Neuron::CreateNewReceptorClaster(double x, double y, double D, _TopologyID TID)
{
	double R = D/2, xr = x - R, yr;
	int s = 1;
	switch (TID) {
		case 0:
			for (int i = 0; i < 9; i++) {
				yr = s*sqrt(fabs(R*R-(xr-x)*(xr-x))) + y;
				CreateNewReceptor(xr, yr);
				if (xr == x + R) {
					s = -1;
				}
				xr += s*(R/2);
			}
			break;
		case 1:
			yr = y - R;
			for (int i = 0; i < 36; i++) {
				CreateNewReceptor(xr, yr);
				xr += D / 5;
				if (xr > x + R) {
					yr += D / 5;
					xr = x - R;
				}
			}
	}

}

float Neuron::SignalsSend(double X, ...)
{
	double *Xs = &X;

	double FiSum = 0, dFiSum = 0, Fi, dFi, R;
	std::pair<double, double> RPos, SPos, FiVector, FiVectorResultant;

	P = 0;

	for (int j = 0; j < Entrys.size(); j++) {
		//double xi = *(Xs+j);
		Entrys[j] -> In(*(Xs+j));
	}
	for (int i = 0; i < Receptors.size(); i++) {
		FiSum = 0;
		dFiSum = 0;
		FiVectorResultant.first = 0;
		FiVectorResultant.second = 0;
		for (int j = 0; j < Entrys.size(); j++) {
			for (int k = 0; k < Entrys[j]->GetSynapsesCount(); k++) {
				_NSynaps *Sk = Entrys[j] -> GetSynaps(k);
				if (Receptors[i]->GetLocked()) RPos = Receptors[i] -> GetPosf();
				else RPos = Receptors[i] -> GetPos();
				SPos = Sk -> GetPos();

				Fi = Parameters -> FiFunction(SPos.first-RPos.first, SPos.second-RPos.second, Sk->Getp(), Sk->GetGamma());
				dFi = Parameters -> FiFunction(SPos.first-RPos.first, SPos.second-RPos.second, Sk->Getp(), Sk->GetdGamma());

				if (i == 14 && Parameters->GetTime() > 1000000) {
					int f = Parameters->GetTime();
					int a = Sk->GetGamma();
					int mu = 0;
				}

				R = 0;
				if (dFi > 0) {
					R = Parameters -> GetFiVectorLength(dFi);
					FiVectorResultant.first += (RPos.first-SPos.first)/sqrt(RPos.first*RPos.first+SPos.first*SPos.first)*R;
					FiVectorResultant.second += (RPos.second-SPos.second)/sqrt(RPos.second*RPos.second+SPos.second*SPos.second)*R;
					dFiSum += dFi;
				}
				FiSum += Fi;
			}
		}

		Receptors[i] -> SetPos(FiVectorResultant.first, FiVectorResultant.second);
		P += Receptors[i] -> CheckActive(FiSum);

		if (Receptors[i]->CheckActive(FiSum)) Receptors[i] -> SetRc(Receptors[i]->GetRc()+dFiSum);
		else Receptors[i] -> SetRc(Receptors[i]->GetRc()/(Receptors[i]->GetRc()*2+1));
	}

	Parameters -> TimeForward();

	P /= Receptors.size();
	int rsize = Receptors.size();
	return P;
}

bool Neuron::SignalReceive()
{
	if (P >= 0.5)
		return 1;
	return 0;
}

void Neuron::CreateCheckpoint() {

	for (int i = 0; i < Receptors.size(); i++) Receptors[i] -> SavePos();
}

void Neuron::ReInit()
{
	for (int i = 0; i < Receptors.size(); i++) Receptors[i] -> DoLock();

	Parameters -> TimeZero();

	for (int i = 0; i < Entrys.size(); i++) {
		Entrys[i] -> ClearSignal();
		for (int j = 0; j < Entrys[i]->GetSynapsesCount(); j++) {
			_NSynaps *Sk = Entrys[i] -> GetSynaps(j);
			Sk -> ClearGamma();
		}
	}
}

std::vector<double> Neuron::CompareCP() {
	std::vector<double> R, CPR;
	std::vector< std::pair<double, double> > CP, CPf;

	for (int i = 0; i < Receptors.size(); i++) {
		if (Receptors[i]->GetLocked()) {
			CP = Receptors[i] -> GetCP();
			CPf = Receptors[i] -> GetCPf();
			int CPC = CP.size();
			if (!i) R = Parameters -> CompareCPFunction(CP, CPf);
			else {
				CPR = Parameters -> CompareCPFunction(CP, CPf);
				for (int j = 0; j < R.size(); j++) R[j] += CPR[j];
			}
		}
	}

	for (int j = 0; j < R.size(); j++) R[j] /= Receptors.size();
	return R;
}

double Neuron::CompareSignals() {
	return CompareSignals(0);
}

double Neuron::CompareSignals(bool WCP)
{
	std::pair<double, double> RPos, RPosf;
	std::vector< std::pair<double, double> > CP, CPf;
	double Result = 0;

	for (int i = 0; i < Receptors.size(); i++) {
		if (Receptors[i]->GetLocked()) {
			RPos = Receptors[i] -> GetPos();
			RPosf = Receptors[i] -> GetPosf();
			CP = Receptors[i] -> GetCP();
			CPf = Receptors[i] -> GetCPf();
			int L = CP.size();
			if (CPf.size() < L) L = CPf.size();
			double Rc = Parameters -> CompareFunction(RPos.first-RPosf.first, RPos.second-RPosf.second, Receptors[i]->Getl(), Receptors[i]->Getlf());
			if (WCP) Rc += Parameters -> CompareCPFunctionD(CP, CPf);
			//Rc /= L + 1;
            Result += Rc;
		}
	}

	Result /= Receptors.size();
	return Result;
}

std::vector<_NEntry*> Neuron::GetEntrys()
{
	return Entrys;
}

std::vector<_NReceptor*> Neuron::GetReceptors()
{
	return Receptors;
}

unsigned int Neuron::GetEntrysCount()
{
	return Entrys.size();
}

unsigned int Neuron::GetSynapsesCount()
{
	unsigned int SSum = 0;
	for (int i = 0; i < Entrys.size(); i++) SSum += Entrys[i] -> GetSynapsesCount();
	return SSum;
}

unsigned int Neuron::GetReceptorsCount()
{
	return Receptors.size();
}

Neuron::~Neuron()
{

}
//---------------------------------------------------------------------------
_NEntry::_NEntry()
{
	Parameters = NULL;
	X = NULL;
}

_NEntry::_NEntry(_NEntry *E, _NParameters *_Parameters)
{
	Parameters = _Parameters;
	for (int i = 0; i < E->GetSynapsesCount(); i++) {
		_NSynaps *S = new _NSynaps(E->GetSynaps(i), Parameters);
		Synapses.push_back(S);
	}
}

_NEntry::_NEntry(_NParameters *_Parameters)
{
	Parameters = _Parameters;
	X = NULL;
}

void _NEntry::AddSynaps(double x, double y, unsigned int Tl, int Type)
{
	_NSynaps *S = new _NSynaps(Parameters, x, y, Tl, Type);
	Synapses.push_back(S);
}

void _NEntry::In(double X)
{
	Signal.push_back(X);

	unsigned int STl = 0, t = Parameters -> GetTime();

	for (int i = 0; i < Synapses.size(); i++) {
        STl = Synapses[i] -> GetTl();
		if (t > STl) Synapses[i] -> In(Signal[t-STl-1]);
		else Synapses[i] -> In(0);
	}
}

void _NEntry::ClearSignal()
{
	Signal.clear();
}

_NSynaps* _NEntry::GetSynaps(unsigned int SID)
{
	return Synapses[SID];
}

unsigned int _NEntry::GetSynapsesCount()
{
	return Synapses.size();
}

_NEntry::~_NEntry()
{

}
//---------------------------------------------------------------------------
_NSynaps::_NSynaps()
{
	Parameters = NULL;
	p = 0;
	Gamma = 0;
	dGamma = 0;
	Type = 0;
}

_NSynaps::_NSynaps(_NSynaps *S, _NParameters *_Parameters) {
	Parameters = _Parameters;
	x = S->GetPos().first;
	y = S->GetPos().second;
	Tl = S -> GetTl();
	Type = S -> GetType();
	p = S -> Getp();
	Gamma = S -> GetGamma();
	dGamma = S -> GetdGamma();
}

_NSynaps::_NSynaps(_NParameters *_Parameters, double _x, double _y, unsigned int _Tl, int _Type)
{
	Parameters = _Parameters;
	x = _x;
	y = _y;
	Tl = _Tl;
	Type = _Type;
	p = Parameters -> pFunction(x, y);
	Gamma = 0;
	dGamma = 0;
}

void _NSynaps::In(double X)
{
	double nGamma = Parameters -> GammaFunction(Gamma, X, Tl, Type);
	dGamma = nGamma - Gamma;
	Gamma = nGamma;
}

void _NSynaps::ClearGamma()
{
	Gamma = 0;
	dGamma = 0;
}

unsigned int _NSynaps::GetTl()
{
	return Tl;
}

int _NSynaps::GetType()
{
	return Type;
}

double _NSynaps::Getp()
{
	return p;
}

double _NSynaps::GetGamma()
{
	return Gamma;
}

double _NSynaps::GetdGamma()
{
	return dGamma;
}

std::pair<double, double> _NSynaps::GetPos()
{
	std::pair<double, double> Pos;
	Pos.first = x;
	Pos.second = y;
	return Pos;
}

_NSynaps::~_NSynaps()
{

}
//---------------------------------------------------------------------------
_NReceptor::_NReceptor()
{
	Parameters = NULL;
	rc = 0.01;
	l = 0;
	lf = 0;
	Locked = 0;
}

_NReceptor::_NReceptor(_NReceptor *R, _NParameters *_Parameters) {
	Parameters = _Parameters;
	Locked = R -> GetLocked();
	x = R->GetPos().first;
	y = R->GetPos().second;
	x0 = R->GetPos0().first;
	y0 = R->GetPos0().second;
	xf = R->GetPosf().first;
	yf = R->GetPosf().second;
	l = R -> Getl();
	lf = R -> Getlf();
	rc = R -> GetRc();
	CP = R -> GetCP();
	CPf = R -> GetCPf();
}

_NReceptor::_NReceptor(_NParameters *_Parameters, double _x, double _y)
{
	Parameters = _Parameters;
	x = _x;
	y = _y;
	x0 = _x;
	y0 = _y;
	xf = _x;
	yf = _y;
	rc = 0.1;
	l = 0;
	lf = 0;
	Locked = 0;
}

bool _NReceptor::CheckActive(double Fi)
{
	if (Fi >= rc) return 1;
	return 0;
}

void _NReceptor::DoLock()
{
	Locked = 1;
	xf = x0;
	yf = y0;
	lf = 0;
	rc = 0.1;
	CPf.clear();
}

void _NReceptor::DoUnlock()
{
	Locked = 0;
}

bool _NReceptor::GetLocked()
{
	return Locked;
}

void _NReceptor::SetPos(double _x, double _y)
{
	if (Locked) {
		xf += _x;
		yf += _y;
		lf += sqrt(_x*_x+_y*_y);
	} else {
		x += _x;
		y += _y;
		l += sqrt(_x*_x+_y*_y);
	}
}

void _NReceptor::SavePos()
{
	if (Locked) {
		if (CPf.size() && CPf.back().first == xf && CPf.back().second == yf) return;
		CPf.push_back(std::make_pair<>(xf, yf));
	} else {
		if (CP.size() && CP.back().first == x && CP.back().second == y) return;
		CP.push_back(std::make_pair<>(x, y));
    }
}

void _NReceptor::SetRc(double _rc) {
	rc = _rc;
}

std::pair<double, double> _NReceptor::GetPos()
{
	std::pair<double, double> Pos;
	Pos.first = x;
	Pos.second = y;
	return Pos;
}

std::pair<double, double> _NReceptor::GetPos0()
{
	std::pair<double, double> Pos;
	Pos.first = x0;
	Pos.second = y0;
	return Pos;
}

std::pair<double, double> _NReceptor::GetPosf()
{
	std::pair<double, double> Pos;
	Pos.first = xf;
	Pos.second = yf;
	return Pos;
}

std::vector< std::pair<double, double> > _NReceptor::GetCP() {
	return CP;
}

std::vector< std::pair<double, double> > _NReceptor::GetCPf() {
	return CPf;
}

double _NReceptor::Getl()
{
	return l;
}

double _NReceptor::Getlf()
{
	return lf;
}

double _NReceptor::GetRc() {
	return rc;
}

_NReceptor::~_NReceptor()
{

}
//---------------------------------------------------------------------------
_NParameters::_NParameters()
{
    t = 0;
	Xm = 128;
	Ym = 128;
	k1 = 4;
	k2 = 0.01;
	Lbd = 0.5;
	D = 0.5;
}

_NParameters::_NParameters(_NParameters *P) {
	t = P -> GetTime();
	Xm = P->GetArea().first;
	Ym = P->GetArea().second;
	k1 = P->Getk1();
	k2 = P->Getk2();
	k3 = P->Getk3();
	Lbd = P->GetLbd();
}

_NParameters::_NParameters(unsigned int _Xm, unsigned int _Ym, double _k1, double _k2, double _Lbd)
{
	t = 0;
	Xm = _Xm;
	Ym = _Ym;
	k1 = _k1;
	k2 = _k2;
	Lbd = _Lbd;
	D = 0.15;
}

double _NParameters::pFunction(double xs, double ys)
{
	double p = 0, h = 1;
	for (double x = 0; x < Xm; x += h) {
		for (double y = 0; y < Ym; y += h) {
			p += Lbd * exp(-Lbd*sqrt((xs-x)*(xs-x)+(ys-y)*(ys-y))) * h * h;
		}
	}
	return p;
}

double _NParameters::GammaFunction(double oG, double Xt, unsigned int Tl, int Type)
{
	double nGamma;
	if (!Type) {
   		nGamma = oG + (k1*Xt - (1-Xt)*oG*k2);
	} else {
		nGamma = oG + (k1*(1-Xt) - Xt*oG*k2);
    }

	return nGamma;
}

double _NParameters::FiFunction(double dx, double dy, double p, double Gamma)
{
	return 1/p * Gamma * Lbd * exp(-Lbd*sqrt(dx*dx+dy*dy));
}

std::vector<double> _NParameters::CompareCPFunction(std::vector< std::pair<double, double> > CP, std::vector< std::pair<double, double> > CPf)
{
	std::vector<double> R;
	int L = CP.size();
	if (CPf.size() < L) L = CPf.size();
	for (int i = 0; i < L; i++) {
		R.push_back(sqrt((CP[i].first-CPf[i].first)*(CP[i].first-CPf[i].first)+(CP[i].second-CPf[i].second)*(CP[i].second-CPf[i].second)));
	}
	return R;
}

double _NParameters::CompareCPFunctionD(std::vector< std::pair<double, double> > CP, std::vector< std::pair<double, double> > CPf)
{
	double R;
	int L = CP.size();
	if (CPf.size() < L) L = CPf.size();
	for (int i = 0; i < L; i++) {
		R += sqrt((CP[i].first-CPf[i].first)*(CP[i].first-CPf[i].first)+(CP[i].second-CPf[i].second)*(CP[i].second-CPf[i].second));
	}
	return R;
}

double _NParameters::CompareFunction(double dx, double dy, double l, double lf)
{
	double R = sqrt(dx*dx+dy*dy), Delta = D*l, dl = fabs(l-lf);
	return R;
}

double _NParameters::GetFiVectorLength(double dFi)
{
	double F = sqrt(dFi);
	return F;
}

std::pair<double, double> _NParameters::GetFiVector(double xs, double ys, double xr, double yr, double R)
{
	double x, y;
	std::pair<double, double> P;

	if (fabs(xs-xr) < 1e-1) {
		P.first = 0;
		if (yr > ys) P.second = R;
		else P.second = -R;
		return P;
	}

	if (fabs(ys-yr) < 1e-1) {
		if (xr > xs) P.first = R;
		else P.first = -R;
		P.second = 0;
		return P;
	}

	double k = (yr-ys) / (xr-xs);
	double b = -k*xs + ys;

	double ax = k*k + 1;
	double bx = 2*(b*k - xr - k*yr);
	double cx = b*b - 2*b*yr + xr*xr + yr*yr - R*R;
	double D1 = bx * bx;
	double D2 = 4 * ax * cx;
	double D = D1 - D2;

	if (D < 0) D = 0;

	if (xs-xr > 0) x = (-bx-sqrt(D)) / 2 / ax;
	else x = (-bx+sqrt(D)) / 2 / ax;

	y = k * x + b;

	P.first = x - xr;
	P.second = y - yr;

	return P;
}

void _NParameters::TimeZero()
{
	t = 0;
}

void _NParameters::TimeForward()
{
	t++;
}

void _NParameters::SetArea(unsigned int _Xm, unsigned int _Ym) {
	Xm = _Xm;
	Ym = _Ym;
}

void _NParameters::Setk1(double _k1) {
	k1 = _k1;
}

void _NParameters::Setk2(double _k2) {
	k2 = _k2;
}

void _NParameters::Setk3(double _k3) {
	k3 = _k3;
}

void _NParameters::SetLbd(double _Lbd) {
	Lbd = _Lbd;
}

unsigned int _NParameters::GetTime()
{
	return t;
}

std::pair<unsigned int, unsigned int> _NParameters::GetArea() {
	std::pair<unsigned int, unsigned int> Area;
	Area.first = Xm;
	Area.second = Ym;
	return Area;
}

double _NParameters::Getk1()
{
	return k1;
}

double _NParameters::Getk2()
{
	return k2;
}

double _NParameters::Getk3()
{
	return k3;
}

double _NParameters::GetLbd()
{
	return Lbd;
}

_NParameters::~_NParameters()
{

}
//---------------------------------------------------------------------------

