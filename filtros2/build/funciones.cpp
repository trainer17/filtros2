#include <cmath>

#include "funciones.h"

//Recibe un número en el rango 0-1 y lo pasa a dB con referencia "1". Tomo 20 por ser "amplitudes"
// Maximo: + 20
// Minimo: -60
float normalized_to_dB(float normalizedValue) {
    return 80. * normalizedValue - 60.;
    //return normalizedValue*20. - (1.-normalizedValue)*60.;
}

//Recibe dB ref a 1
float db_to_linear(float db) {
    return std::pow(10., db/20.);
}



//Recibe un numero en el rango 0-1 y lo pasa a frecuencia 20hz-20khz, pero con movimiento exponencial
float normalized_to_log_freq(float normalizedValue) {
    return 20 * std::pow(1000., normalizedValue);
}

//Recibe un numero en el rango 0-1 que representa la cantidad de 3 octavas relativas a fc, y lo pasa a hz
// Ej:    valor = 0.3333, representa 1 octava respecto a fc.  Si fc = 100hz,  bw = 33,33hz   (67 - 133hz)
// Que sea  control en octavas es una heuristica aproximada... tendria que ser asimetrica en ese caso. Aca no lo hago asi

//Cuenta:   (f + kf) / (f- kf) = 2^n_oct   --> k = (2^n -1) / (2^n +1)    .   bw = kf
// 
// En ese ejemplo:  n = 1 --> k = 1/3 --> bw = 100/3 =  33,33
// Si valor = 1/6, k = raiz(2) -1 / raiz(2) +1 = 0.1715 --> bw = 17.15
float normalized_octs_to_hz_bw(float normalizedValue, float fc, float sr) {


    float n_oct = normalizedValue * 3;
    float k = (std::pow(2., n_oct) - 1) / (std::pow(2., n_oct) + 1);

    float bw_hz = fc * k;

    //Clamp bandwith para que no se pase de Nyquist ni a frecuencias negativas
    if (fc + bw_hz > sr / 2.) bw_hz = sr / 2 - fc - 2; //al limite con nyquist
    if (fc - bw_hz < 0) bw_hz = fc - 2; // al limite con 0hz

    return  bw_hz;

}



//Calcula el gain en la frecuencia de corte según el criterio "3db abajo del pico" cuando el gain es mayor a 3dB (sqrt(2)).
float cutoff_freq_Gain(float G2, float G02) {

	float Gb2;

	//Boost
	if (G2 >= G02) {

		if (G2 > 2 * G02) Gb2 = G2 / 2.;
		else  Gb2 = (G2 + G02) / 2.;
	}

	//Cut
	if (G2 < G02) {

		if (G2 < G02 / 2.) Gb2 = 2. * G2;
		else Gb2 = (G2 + G02) / 2.;
	}

	return Gb2;

}


int normalized_to_Orden(float valueNormalized, int ordenMax) {
	return 1 + static_cast<int>(valueNormalized * (ordenMax - 0.001));  // Ensures 1–250
}