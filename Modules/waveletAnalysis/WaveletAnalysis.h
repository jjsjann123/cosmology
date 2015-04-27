#pragma once
#include <math.h>
#include <vector>

using namespace std;

class CFilterFunc{
protected:
	vector<double> mFunc;
	int begin;
	int end;

public:
	CFilterFunc(const vector<double> &input):mFunc(input){
		begin = 1 - (mFunc.size() + 1)/2;/*0;*/
		end = mFunc.size()/2 + 1;/*mFunc.size();*/
	}

	int ibegin(){return begin;}
	int iend(){return end;}

	unsigned int size() { return mFunc.size(); }

	const vector<double> & getFunc(){return mFunc;}
	inline double getValue(int offset){
		return mFunc[offset - ibegin()];
	}
};

class WaveletAnalysis{
public:
	WaveletAnalysis(vector<double> input,vector<unsigned int> dim,unsigned int level,
		vector<double> h0,vector<double> h1,vector<double> h2,vector<double> h3);
	~WaveletAnalysis(void);

private:
	CFilterFunc funcH0,funcH1,funcH2,funcH3;

	unsigned int level;
	vector<double> scalevalues;
	vector<double> waveletvalues;

	vector<double> *inputvalues;
	vector<double> *outputvalues;
	vector<unsigned int> offsets;
	vector<unsigned int> lens;
	vector<unsigned int> dimensions;

public:
	void resetlevel(unsigned int level);
	void resetinput(vector<double> input,vector<unsigned int> ds);

	void transform();
	void itransform();

	double getOutputValue(vector<unsigned int> ord){
		offsets = ord;
		unsigned int offset = getOffset();
		return (*outputvalues)[offset];}

	void process();
private:
	typedef void (WaveletAnalysis::*FuncInWhile)(void *);
	void whileProcess(FuncInWhile func,void* parameter);

	void initOffsetsAndLens(unsigned int level);
	void initOutput(void *v);

	void reconstructOneDimension(void * d);
	void analyzeOneDimension(void * d);

	inline void copyInput2Output(void *);
	inline void copyInput2OuputOnDim(void *dim);

	inline void swapVectorPointer();
	unsigned int getOffset();
};
