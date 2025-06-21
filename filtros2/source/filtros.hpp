//------------------------------------------------------------------------
// Copyright(c) 2025 ASKA.
//------------------------------------------------------------------------

#pragma once

typedef float* ssignal;
#include <vector>
#include "processor.h"
#include "cids.h"
#include "circularBuff.hpp"
#include <cmath>
#include <stdexcept>


#ifndef M_PI
#   define M_PI 3.1415926535897932384626433832
#endif

template<typename Sample>
struct Filtro {

	std::vector<Sample> as;
	std::vector<Sample> bs;

	void filter(circularBuffer<Sample>& in, circularBuffer<Sample>& out, int nsamps);

	void calcularCoeffs(int tipoId, float fc, float sr, float bw, int orden); //Le paso todo lo posible y despues segun el tipo veo que necesito usar

	void calcularCoeffsNotch2(float w,  float dw);
	void calcularCoeffsResonator2(float w,  float dw);
	void calcularCoeffsParametricEq2(float w,  float dw);
	void calcularCoeffsShelve2(float w, float G);

};

//Hago convolucion por definición, sin FFT, porque asumo pocos coeficientes
//Meto nsamps filtrados en out[i; i+nsamps) , tomando a partir de in[i:]
template<typename Sample>
void Filtro<Sample>::filter(circularBuffer<Sample>& in, circularBuffer<Sample>& out, int nsamps) {

	int ns = nsamps; //copia

	while (--nsamps >= 0) {

		Sample yn = 0.;
		for (int k = 0; k < bs.size(); k++) {
			yn += in[-k] * bs[k];  //xk
		}

		for (int k = 1; k < as.size(); k++) {
			yn -= out[-k] * as[k]; //a0 es forzado a 1. Notar el signo menos
		}

		out.next() = yn;
		in.advance();  //avanzo los indices del ultimo sample

	}

	//rebobino para que el indice actual del buffer siga siendo el mismo que cuando llamaste
	out.rewind(ns);
	in.rewind(ns);

	return;

}

template<typename Sample>
void Filtro<Sample>::calcularCoeffs(int tipoId, float fc, float sr, float bw, int orden) {


	Sample w = 2 * M_PI * fc / sr; //Hz a radianes
	bw = 2 * M_PI * bw / sr; //paso Hz a radianes


	switch (tipoId)
	{
	case TiposFiltro::Butterworth:

		break;

	case TiposFiltro::Chebyshev:

		break;

	case TiposFiltro::Elliptic:

		break;


	case TiposFiltro::Notch2:
		calcularCoeffsNotch2(w, bw);
		break;


	case TiposFiltro::Resonator2:

		calcularCoeffsResonator2(w, bw);

		break;
	case TiposFiltro::ParametricEq2:
		calcularCoeffsParametricEq2(w, bw);
	}
	
	case TiposFiltro::ParametricEq2:
		calcularCoeffsShelve2(w, G);
	}

	return;

}


//Filtro Resonador orden 2: All pole. Solo dos polos
// yn / G = a1 y_n_1 + a2 y_n_2
// Polo:   R.exp(i*w),  w entre 0-pi
// a2 = R^2 ; a1 = -2 R cos(w)
// b0 = G = (1-R ) sqrt(1 - 2R cos(w) + R2) 

template<typename Sample>
void Filtro<Sample>::calcularCoeffsResonator2(float w, float bw) {

	as.resize(3);
	bs.resize(1);

	Sample R = 1 - bw / 2;
	Sample G = (1 - R) * std::sqrt(1 - 2 * R * cos(w) + R * R);

	as[2] = R * R;
	as[1] = -2 * R * cos(w);
	bs[0] = G;

	//Chequeos de seguridad
	if(R< 0 || R>1) throw std::invalid_argument("¡Error en el calculo de R!");
	if(w < 0 || w>M_PI) throw std::invalid_argument("¡Error en el calculo de w!");
	if(G < 0) throw std::invalid_argument("¡Error en el calculo de G!");

}

template<typename Sample>
void Filtro<Sample>::calcularCoeffsParametricEq2(float w, float bw) {


	//Chequeos de seguridad
	//Todo

}


//Filtro Notch orden 2: All pole. Solo dos polos 
template<typename Sample>
void Filtro<Sample>::calcularCoeffsNotch2(float w, float bw) {

	as.resize(3);
	bs.resize(3);

	Sample Gb = 1. / std::sqrt(2); //Gain en las freqs de corte

	Sample beta = std::sqrt(1 - Gb * Gb) / Gb * std::tan(bw / 2);

	Sample b = 1. / (1. + beta);

	bs[0] = b;
	bs[1] = -2 * b * std::cos(w);
	bs[2] = b;


	as[2] = 2 * b - 1;
	as[1] = -2 * b * std::cos(w);
	
}


template<typename Sample>
void Filtro<Sample>::calcularCoeffsShelve2(float w, float G) {

	as.resize(3);
	bs.resize(3);

	Sample Gb = 1. / std::sqrt(2); //Gain en las freqs de corte

	Sample beta = std::sqrt(1 - Gb * Gb) / Gb * std::tan(bw / 2);

	Sample b = 1. / (1. + beta);

	bs[0] = b;
	bs[1] = -2 * b * std::cos(w);
	bs[2] = b;


	as[2] = 2 * b - 1;
	as[1] = -2 * b * std::cos(w);

}