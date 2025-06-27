#pragma once
#include <cmath>
//Recibe un número en el rango 0-1 y lo pasa a dB con referencia "1". Tomo 20 por ser "amplitudes"
float normalized_to_dB(float normalizedValue); 

float db_to_linear(float db);

//Recibe un numero en el rango 0-1 y lo pasa a frecuencia 20hz-20khz, pero con movimiento exponencial
float normalized_to_log_freq(float normalizedValue);

float normalized_octs_to_hz_bw(float normalizedValue, float fc, float sr);

//Calcula el gain en la frecuencia de corte según el criterio "3db abajo del pico" cuando el gain es mayor a 3dB (sqrt(2)).
//Para los filtros eq parametricos y shelves
float cutoff_freq_Gain(float G2, float G02);

//Calcula el orden del filtro. De 1 a 250
int normalized_to_Orden(float normalizedValue, int ordenMax);