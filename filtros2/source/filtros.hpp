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
#include "funciones.h"


#ifndef M_PI
#   define M_PI 3.1415926535897932384626433832
#endif


// Nth-order cascading section. Tipicamente va a ser de orden dos, pero puede ser mayor
template<typename Sample>
struct Cascade_section {
	std::vector<Sample> as;
	std::vector<Sample> bs;
};


template<typename Sample>
class Filtro {

public:
	
	void filter(circularBuffer<Sample>& in, circularBuffer<Sample>& out, int nsamps);

	void calcularCoeffs(int tipoId, float fc, float sr, float bw, int orden, float Gain); //Le paso todo lo posible y despues segun el tipo veo que necesito usar

	Filtro();

private:


	//Cascadas del filtro (secciones de orden arbitrario. La idea es que sean de orden 2),
	std::vector<Cascade_section<Sample>> cascading_sections;


	//Referencia a los coefs de la PRIMER SECCION del filtro. Usado por conveniencia, porque todos los filtros de orden bajo solo usan una seccion
	

	std::vector<Sample>& as;
	std::vector<Sample>& bs;

	Sample gain; //factor de gain que se le aplica al yn.
				// Lo uso cuando todos los coeficientes salvo el de yn llevan un mismo factor, que lo paso al otro lado de la ec. 
				// H(z) =  G ( b0 + b1z-1 +..) / 1 + G a1 z-1 + G a2 z-2 ...
	             // Multiplico por G a yn al final de todo
				//TODO: No lo estoy haciendo aun. Pero en Butterworth y otros filtros podria. Si mejorara la precisión numérica

	//Le paso los coeficientes y filtra
	void filterSection(circularBuffer<Sample>& in, circularBuffer<Sample>& out, int nsamps, int section); // Secciones del filtro

	void calcularCoeffsLP(float w, float Gain);
	void calcularCoeffsHP(float w, float Gain);
	void calcularCoeffsNotch2(float w,  float dw, float Gb = 1. / std::sqrt(2));
	void calcularCoeffsResonator2(float w, float dw, float Gb = 1./ std::sqrt(2));
	void calcularCoeffsAllPoleResonator2(float w,  float dw);
	void calcularCoeffsParametricEq2(float w, float G,  float dw,  float G0 = 1.);
	void calcularCoeffsShelve2(float w, float G, bool lowshelve);

	void calcularCoeffsButterworth(float w, int orden, float G, float Gcut = 1/std::sqrt(2));

};


//Todo : El filtro deberia tener tantas copias del input buffer como secciones en la cascada
// Esto es para poder preservar el estado inicial/anterior en cada seccion (los primeros y-1, y-2, y-3... y-orden, que en el metodo de "usar un solo buffer e ir copiando a la salida" no anda
template<typename Sample>
Filtro<Sample>::Filtro() :
	cascading_sections(1),
	as(cascading_sections[0].as),
	bs(cascading_sections[0].bs)
	{
		as.resize(2);
		bs.resize(2);
		bs[0] = 1.;

		return;
	}

//Puedo tener una sola seccion, pero para generalizar lo pongo asi
template<typename Sample>
void Filtro<Sample>::filter(circularBuffer<Sample>& in, circularBuffer<Sample>& out, int nsamps) {

	filterSection(in, out, nsamps, 0); //Primer seccion (seccion 0)

	//Secciones 1 en adelante si hay. Necesitan mirar el output de la seccion anterior, por eso el primero lo hago aislado
	for (int section = 1; section < cascading_sections.size(); section++) {

		.
		in.copyfromIn(out, nsamps); //la salida del anterior es la entrada del siguiente filtro
		filterSection(in, out, nsamps, section);
	
	}
	return;
}


//Hago convolucion por definición usando los as y bs, sin FFT, porque asumo pocos coeficientes
//Meto nsamps filtrados en out[i; i+nsamps) , tomando a partir de in[i:]

//Filtro una seccion de la cascada
template<typename Sample>
void Filtro<Sample>::filterSection(circularBuffer<Sample>& in, circularBuffer<Sample>& out, int nsamps, int section) {

	std::vector<Sample>& ass = cascading_sections[section].as;
	std::vector<Sample>& bss = cascading_sections[section].bs;


	int ns = nsamps; //copia

	while (--ns >= 0) {

		Sample yn = 0.;
		for (int k = 0; k < bss.size(); k++) {
			yn += in[-k] * bss[k];  //xk
		}

		for (int k = 1; k < ass.size(); k++) {
			yn -= out[-k] * ass[k]; //a0 es forzado a 1. Notar el signo menos
		}

		out.next() = yn;
		in.advance();  //avanzo los indices del ultimo sample

	}

	//rebobino para que el indice actual del buffer siga siendo el mismo que cuando llamaste
	out.rewind(nsamps);
	in.rewind(nsamps);

}


template<typename Sample>
void Filtro<Sample>::calcularCoeffs(int tipoId, float fc, float sr, float bw, int orden, float Gain) {


	Sample w = 2 * M_PI * fc / sr; //Hz a radianes
	bw = 2 * M_PI * bw / sr; //paso Hz a radianes


	switch (tipoId)
	{
	case TiposFiltro::Butterworth:
		calcularCoeffsButterworth(w, orden, Gain);

		break;

	case TiposFiltro::Chebyshev:

		break;

	case TiposFiltro::Elliptic:

		break;

	case TiposFiltro::LP:
		calcularCoeffsLP(w, Gain);
		break;

	case TiposFiltro::HP:
		calcularCoeffsHP(w, Gain);

		break;


	case TiposFiltro::Notch2:
		calcularCoeffsNotch2(w, bw, 1. / std::sqrt(2));
		break;


	case TiposFiltro::Resonator2:
		calcularCoeffsResonator2(w, bw, 1. / std::sqrt(2));
		break;


	case TiposFiltro::AllPoleResonator2:
		calcularCoeffsAllPoleResonator2(w, bw);
		break;

	case TiposFiltro::ParametricEq2:
		calcularCoeffsParametricEq2(w, Gain, bw);
		break;

	case TiposFiltro::LowShelveEq2:
		calcularCoeffsShelve2(w, Gain, true);
		break;

	case TiposFiltro::HighShelveEq2:
		calcularCoeffsShelve2(w, Gain, false);
		break;
	}

	return;

}

//TODO KNEE

template<typename Sample>
void Filtro<Sample>::calcularCoeffsLP(float w, float Gc) {

	as.resize(2);
	bs.resize(2);

	if (Gc >= 1.)  return; //filtroBypass(2);

	Sample alpha = Gc / std::sqrt(1 - Gc * Gc) * std::tan(w/2);
	
	Sample b = alpha / (alpha + 1);  // = (1-a)/2;
	as[1] = (alpha - 1.) / (alpha + 1.);
	bs[0] = b;
	bs[1] = b;
}


template<typename Sample>
void Filtro<Sample>::calcularCoeffsHP(float w, float Gc) {

	as.resize(2);
	bs.resize(2);

	if (Gc >= 1.)  return; //Habria que poner un limite de rango al control en este tipo de filtro. pero wenn

	Sample alpha =  std::sqrt(1 - Gc * Gc) /Gc * std::tan(w / 2);

	Sample b = 1. / (alpha + 1);  // = (1+a)/2;
	as[1] = (alpha - 1.) / (alpha + 1.);
	bs[0] = b;
	bs[1] = -b;


}


//Filtro Resonador orden 2: All pole. Solo dos polos
// yn / G = a1 y_n_1 + a2 y_n_2
// Polo:   R.exp(i*w),  w entre 0-pi
// a2 = R^2 ; a1 = -2 R cos(w)
// b0 = G = (1-R ) sqrt(1 - 2R cos(w) + R2) 

template<typename Sample>
void Filtro<Sample>::calcularCoeffsAllPoleResonator2(float w, float bw) {

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
void Filtro<Sample>::calcularCoeffsResonator2(float w, float bw, float Gb) {

	as.resize(3);
	bs.resize(3);

	float beta = Gb / std::sqrt(1. - Gb * Gb) * std::tan(bw / 2.);

	float b = 1. / (1. + beta);

	as[2] = 2*b-1;
	as[1] = -2 * b * cos(w);
	bs[0] = 1-b;
	bs[1] = 0.;
	bs[2] = b-1;

}



template<typename Sample>
void Filtro<Sample>::calcularCoeffsParametricEq2(float w, float G, float bw, float G0) {

	as.resize(3);
	bs.resize(3);

	float G2 = G * G;
	float G02 = G0 * G0;

	if (G == 1.) //caso degenerado, pero puede pasar al inicializar el plugin y eso
	{
		as[1] = as[2] = 0;
		bs[1] = bs[2] = 0.;
		return;
	}

	float Gb2 = cutoff_freq_Gain(G2, G02); //Este es el gain en la frecuencia de corte. Es parametrizable, daria mas libertad elegirlo por el usuario, pero tipicamente se elige como un valor no controlable


	float beta = std::sqrt((Gb2 - G02) / (G2 - Gb2)) * std::tan(bw/2.);  ///CUIDADO: CASOS BORDE. DIvision por cero

	float beta1 = 1 + beta;

	bs[0] = (G0 + G * beta) / beta1;
	bs[1] = -2*G0*std::cos(w) / beta1;
	bs[2] = (G0 - G * beta) / beta1;

	as[1] = -2 * std::cos(w)/ beta1;
	as[2] = (1 - beta) / beta1;


}

//TODO Revisar BW
//Filtro Notch orden 2:
template<typename Sample>
void Filtro<Sample>::calcularCoeffsNotch2(float w, float bw, float Gb) {

	as.resize(3);
	bs.resize(3);

 
	Sample beta = std::sqrt(1 - Gb * Gb) / Gb * std::tan(bw / 2);

	Sample b = 1. / (1. + beta);

	bs[0] = b;
	bs[1] = -2 * b * std::cos(w);
	bs[2] = b;


	as[2] = 2 * b - 1;
	as[1] = -2 * b * std::cos(w);
	
}


template<typename Sample>
void Filtro<Sample>::calcularCoeffsShelve2(float w, float G, bool lowshelf) {


	as.resize(2);
	bs.resize(2);

	//G es el parametro lineal ya

	float G0 = 1.; //Acá no veo la necesidad de que sea un parámetro

	float G02 = 1.; //G0 al cuadrado
	//float Gc2 = cutoff_freq_Gain(G*G, G02); //Este es el gain en la frecuencia de corte. Es parametrizable, daria mas libertad elegirlo por el usuario, pero tipicamente se elige como un valor no controlable

	//float beta = std::sqrt((Gc2 - G02) / (G * G - Gc2));  ///CUIDADO: CASOS BORDE. DIvision por cero
	float beta = 1; //Opcion canónica para Gc

	if(lowshelf) beta *= std::tan(w / 2.);
	else beta *=  1./ std::tan(w / 2.);

	float beta1 = 1 + beta;

	bs[0] =   (G0 + G * beta) /beta1;
	bs[1] =   (G0 - G * beta) / beta1;

	as[1] = (1 - beta) / beta1;

	if (lowshelf) {
		as[1] *= -1;
		bs[1] *= -1;
	}

}


//Especifico el orden
// No uso los datos de gain
//Podria tambien especificar los parametros completos: Gain en la banda de paso, atenuacion en la banda de rechazo, y abrupticidad de la transicion para calcular el orden en funcion de esos parametros
template<typename Sample>
void Filtro<Sample>::calcularCoeffsButterworth(float w, int orden, float G, float Gcut) {

	//Gcut : Gain en la freq de corte. Tomada como 1/sqrt(2) por defecto
	
	/*
	if (G >= 1.) return; //El filtro solo atenua

	Sample eps_pass = std::sqrt(1. / G_cut - 1);
	Sample eps_stop = std::sqrt(1. / G - 1);

	int orden = 6;
	*/

	Sample W0 = 2*w; //En realidad es otro ratio, pero no lo construyo con esos parámetros
	Sample W02 = W0 * W0;

	int K = orden / 2; // la cantidad de secciones de orden 2.

	cascading_sections.resize(K);
	
	for (int i = 0; i < K; i++)
	{

		Sample thetai = M_PI  * (orden + 1 + 2 * i) / (2. * orden);

		Sample denominador = (1. - 2 * W0 * std::cos(thetai) + W02); //TOdo esto puede ponerse como factor de Gain del filtro, ya que todos los coeficientes lo llevan. 
		Sample denominadorConj= (1. + 2 * W0 * std::cos(thetai) + W02); //TOdo esto puede ponerse como factor de Gain del filtro, ya que todos los coeficientes lo llevan. 


		cascading_sections[i].as.resize(3);
		cascading_sections[i].as[1] = 2 * (W02 - 1.) / denominador ;
		cascading_sections[i].as[2] = denominadorConj / denominador;

		cascading_sections[i].bs.resize(3);
		Sample Gi = W02 / denominador;

		cascading_sections[i].bs[0] = Gi;
		cascading_sections[i].bs[1] = 2*Gi;
		cascading_sections[i].bs[2] = Gi;

	}

	//Falta una seccion de orden uno
	if (orden % 2 == 1) {

		cascading_sections.resize(K + 1);

		cascading_sections[K].as.resize(2);
		cascading_sections[K].bs.resize(2);

		Sample G0 = W0 / (W0 + 1.);

		cascading_sections[K].as[1] = (W0-1.)/(W0+1.);

		cascading_sections[K].bs[0] = G0;
		cascading_sections[K].bs[1] = G0;

	}

	return;

}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/*
template<typename Sample>
void Filtro<Sample>::calcularCoeffsButterworth(float w) {

	return;
}

*/


