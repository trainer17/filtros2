#pragma once
#include <vector>

/// <Circular Buffer
template<typename Sample>
class circularBuffer {

public:
	circularBuffer() : bufsize(0), i(-1) {};
	circularBuffer(int bufsize) : bufsize(bufsize), buf(bufsize), i(0) {};
	~circularBuffer();
	void copytoOut(float* to, int nsamps);
	void copytoOut(circularBuffer<Sample>& to, int nsamps);
	void copyfromIn(float* from, int nsamps);
	void copyfromIn(circularBuffer<Sample>& from, int nsamps);


	int bufsize;
	std::vector<Sample> buf;
	int i; //indice "actual"

	Sample& operator[](int offset); //offset respecto al indice
	void advance(); //avanza i en uno
	void advance(int n); //avanza i en n
	void rewind(int n); //retrocede i en n muestras
	void wrap(); // si i no está en [0;bufsize), lo wrapea a ese intervalo
	Sample& next(); //devuelve el ultimo sample y avanza i en uno

};

template<typename Sample>
circularBuffer<Sample>::~circularBuffer() {

}

template<typename Sample>
Sample& circularBuffer<Sample>::operator[](int offset) {
	int j = i + offset;
	while (j >= bufsize) j -= bufsize; // % no anda bien con los negativos, asi que lo hago de esta manera
	while (j < 0) j += bufsize;
	return buf[j];

}

template<typename Sample>
void circularBuffer<Sample>::wrap() {
	while (i >= bufsize) i -= bufsize;
	while (i < 0) i += bufsize;
	//Como % devuelve negativos, uso esa implementacion propia
}
template<typename Sample>
void circularBuffer<Sample>::advance() {
	i += 1;
	if (i == bufsize) i = 0;
	return;
}

template<typename Sample>
void circularBuffer<Sample>::advance(int n) {
	i += n;
	wrap();
	return;
}

template<typename Sample>
void circularBuffer<Sample>::rewind(int n) {
	i -= n;
	wrap();
	return;
}


template<typename Sample>
Sample& circularBuffer<Sample>::next() {
	Sample& out = buf[i];
	advance();
	return out;
}

template<typename Sample>
void circularBuffer<Sample>::copytoOut(float* to, int nsamps) {
	int ns = nsamps;
	while (--ns >= 0) {
		(*to++) = (*this).next();
	}

	rewind(nsamps);
	return;
}

template<typename Sample>
void circularBuffer<Sample>::copytoOut(circularBuffer<Sample>& to, int nsamps) {
	int ns = nsamps;
	while (--ns >= 0) {
		to.next() = (*this).next();
	}

	rewind(nsamps);
	to.rewind(nsamps);
	return;
}


template<typename Sample>
void circularBuffer<Sample>::copyfromIn(float* from, int nsamps) {

	int ns = nsamps;
	while (--nsamps >= 0) {
		(*this).next() = (*from++);
	}

	rewind(ns);

	return;
}

template<typename Sample>
void circularBuffer<Sample>::copyfromIn(circularBuffer<Sample>& from, int nsamps) {

	int ns = nsamps;
	while (--ns >= 0) {
		(*this).next() = from.next();
	}

	rewind(nsamps);
	from.rewind(nsamps);

	return;
}


