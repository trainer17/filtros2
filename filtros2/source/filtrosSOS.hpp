//------------------------------------------------------------------------
// Copyright(c) 2025 ASKA.
//------------------------------------------------------------------------

#pragma once

typedef float* ssignal;
#include <vector>
#include "processor.h"
#include "cids.h"
#include "circularBuff.hpp"
#include <iostream>

template<typename Sample>
struct SOS {

	std::pair<Sample> as; 
	std::pair<Sample> bs; 
	bool ordenUno = false; //Si es seccion de orden 1, cada segundo del par es 0

};

template<typename Sample>
struct FiltroSOS {

	std::vector<SOS> SOS_as; 
	std::vector<SOS> SOS_bs;

	void filterSOS(circularBuffer<Sample>& in, circularBuffer<Sample>& out, int nsamps);

};






