
#include "WaveletAnalysis.h"
#include <clocale>

inline double Log2(double x) {
	return logf(x) / logf(2.f);
}

WaveletAnalysis::WaveletAnalysis(vector<double> input,vector<unsigned int> dim,unsigned int l,
								 vector<double> h0,vector<double> h1,vector<double> h2,vector<double> h3):
funcH0(h0),
funcH1(h1),
funcH2(h2),
funcH3(h3){
	resetlevel(l);
	resetinput(input,dim);
}

WaveletAnalysis::~WaveletAnalysis(void){

}

void WaveletAnalysis::resetlevel(unsigned int l){
	level = l;
}

void WaveletAnalysis::resetinput(vector<double> input,vector<unsigned int> dim){
	unsigned int alldimlen = 1;
	dimensions.resize(dim.size());

	for(unsigned int i = 0; i < dim.size(); i++){
		int len = 1 << (int)ceil(Log2(dim[i]));
		dimensions[i] = len;
		alldimlen *= len;
	}
	offsets.resize(dimensions.size());
	lens.resize(dimensions.size());

	scalevalues.resize(alldimlen);
	waveletvalues.resize(alldimlen);

	inputvalues = &input;
	outputvalues = &scalevalues;
	initOffsetsAndLens(0);
	whileProcess(&WaveletAnalysis::copyInput2OuputOnDim,&dim);
}

void WaveletAnalysis::transform(){
	inputvalues = &scalevalues;
	outputvalues = &waveletvalues;

	for(unsigned int l = 0; l < level; ++l){
		for(unsigned int d = 0; d < dimensions.size(); d++){
			initOffsetsAndLens(l);
			lens[d] = lens[d] >> 1;
			whileProcess(&WaveletAnalysis::analyzeOneDimension,&d);
			swapVectorPointer();
		}

		initOffsetsAndLens(l);
		whileProcess(&WaveletAnalysis::copyInput2Output,NULL);
		swapVectorPointer();

		initOffsetsAndLens(l+1);
		whileProcess(&WaveletAnalysis::copyInput2Output,NULL);
		swapVectorPointer();
	}
}

void WaveletAnalysis::itransform(){
	for(unsigned int l = level; l > 0; --l){
		for(int d = dimensions.size() - 1; d >=0 ; d--){
			initOffsetsAndLens(l-1);
			whileProcess(&WaveletAnalysis::initOutput,0);

			initOffsetsAndLens(l-1);
			lens[d] = lens[d] >> 1;

			whileProcess(&WaveletAnalysis::reconstructOneDimension,&d);
			swapVectorPointer();

			initOffsetsAndLens(l-1);
			whileProcess(&WaveletAnalysis::copyInput2Output,NULL);
			swapVectorPointer();
		}
	}
	swapVectorPointer();
}

void WaveletAnalysis::analyzeOneDimension(void * par){
	int d = *((int*)par);
	unsigned int offset = 0;
	unsigned int tmpi = offsets[d];
	unsigned int offsetD = offsets[d] << 1;

	double res = 0;
	for(int i = funcH0.ibegin(); i < funcH0.iend(); i++ ){
		int index = i + offsetD;
		offsets[d] = index;
		if (index < 0 || index >= dimensions[d]) continue;

		offset = getOffset();
		res += funcH0.getValue(i) * (*inputvalues)[offset];
	}
	offsets[d] = tmpi;

	offset = getOffset();
	(*outputvalues)[offset] = res;

	res = 0;
	for(int i = funcH1.ibegin(); i < funcH1.iend(); i++ ){
		int index = i + offsetD;
		offsets[d] = index;
		if (index < 0 || index >= dimensions[d]) continue;
		unsigned int offset = getOffset();
		res += funcH1.getValue(i) * (*inputvalues)[offset];
	}
	offsets[d] = tmpi + lens[d];

	offset = getOffset();
	(*outputvalues)[offset] = res;

	offsets[d] = tmpi;
}

void WaveletAnalysis::reconstructOneDimension(void *par){
	int d = *((int*)par);
	unsigned int offset = 0;
	unsigned int tmpi = offsets[d];
	unsigned int offsetD = offsets[d] << 1;

	for(int i = funcH2.ibegin(); i < funcH2.iend(); i++ ){
		int index = i + offsetD;
		if (index < 0 || index >= (lens[d]<<1)) continue;

		offsets[d] = index;
		unsigned int offset1 = getOffset();
		offsets[d] = tmpi;
		unsigned int offset2 = getOffset();

		(*outputvalues)[offset1] += 
			funcH2.getValue(i) * (*inputvalues)[offset2];
	}

	for(int i = funcH3.ibegin(); i < funcH3.iend(); i++ ){
		int index = i + offsetD;
		if (index < 0 || index >= (lens[d]<<1)) continue;
		offsets[d] = index;
		unsigned int offset1 = getOffset();
		offsets[d] = tmpi + lens[d];
		unsigned int offset2 = getOffset();

		(*outputvalues)[offset1] += 
			funcH3.getValue(i) * (*inputvalues)[offset2];
	}
	offsets[d] = tmpi;
}

void WaveletAnalysis::whileProcess(FuncInWhile func,void* parameter){
	bool isOver = false;
	while(!isOver){
		(this->*func)(parameter);
		unsigned int cur = 0;
		while(true){
			offsets[cur]++;
			if (offsets[cur] >= lens[cur]){
				offsets[cur] = 0;
				cur++;
				if (cur >= dimensions.size()){
					isOver = true;
					break;
				}
			}else{ 
				break;
			}
		}
	}
}

void WaveletAnalysis::initOffsetsAndLens(unsigned int level){
	for(unsigned int i = 0; i < dimensions.size(); i++){
		offsets[i] = 0;
		lens[i] = dimensions[i] >> level;
	}
}

void WaveletAnalysis::initOutput(void *par){
	int val = (int)par;
	unsigned int offset = getOffset();
	(*outputvalues)[offset] = val;
}

unsigned int WaveletAnalysis::getOffset(){
	unsigned int offset = 0;
	unsigned int dlenth = 1;

	for(unsigned int i = 0; i < dimensions.size(); ++i){
		offset += offsets[i] * dlenth;
		dlenth *= dimensions[i];
	}
	return offset;
}

inline void WaveletAnalysis::swapVectorPointer(){
	vector<double> *tmp = inputvalues;
	inputvalues = outputvalues;
	outputvalues = tmp;
}

void WaveletAnalysis::copyInput2Output(void *){
	unsigned int offset = getOffset();
	(*outputvalues)[offset] = (*inputvalues)[offset];
}

void WaveletAnalysis::copyInput2OuputOnDim(void * par){
	unsigned int offset1 = getOffset();
	vector<unsigned int> *dim = (vector<unsigned int> *)par;

	unsigned int offset2 = 0;
	unsigned int dlenth = 1;

	for(unsigned int i = 0; i < dimensions.size(); ++i){
		offset2 += offsets[i] * dlenth;
		dlenth *= (*dim)[i];
	}

	if (offset1 < inputvalues->size())
		(*outputvalues)[offset1] = (*inputvalues)[offset2];
}


void WaveletAnalysis::process(){
	swapVectorPointer();
	initOffsetsAndLens(0);
	whileProcess(&WaveletAnalysis::initOutput,0);

	initOffsetsAndLens(2);
	whileProcess(&WaveletAnalysis::copyInput2Output,NULL);
}
