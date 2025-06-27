//------------------------------------------------------------------------
// Copyright(c) 2025 ASKA.
//------------------------------------------------------------------------

#include "controller.h"
#include "cids.h"
#include "base/source/fstreamer.h"
#include "funciones.h"

//Para convertir cualquier string a utf16 (el formato que usa steinberg)
/*
#include <codecvt>
#include <cuchar>
#include <string>
#include <locale>
*/

using namespace Steinberg;

namespace MyCompanyName {


//sobre control de parametros https://forums.steinberg.net/t/clarification-of-parameter-handling-in-vst-3/201914/3
	
//------------------------------------------------------------------------
// filtros2Controller Implementation
//------------------------------------------------------------------------
tresult PLUGIN_API filtros2Controller::initialize (FUnknown* context)
{
	// Here the Plug-in will be instantiated

	//---do not forget to call parent ------
	tresult result = EditControllerEx1::initialize (context);
	if (result != kResultOk)
	{
		return result;
	}

	// Here you could register some parameters
	parameters.addParameter(STR16("Fc"), STR16("Hz"), 0, .5, Vst::ParameterInfo::kCanAutomate, Parametros::kParamFc, 0);
	//A VST 3 parameter is always normalized (its value is a floating point value between [0.0, 1.0]), here its default value is set to 0.5.
	parameters.addParameter(STR16("Orden"), STR16(""), 250, 0.01, Vst::ParameterInfo::kCanAutomate, Parametros::kParamOrden, 0);
	parameters.addParameter(STR16("Tipo"), STR16(""), NTiposFiltro, 0, Vst::ParameterInfo::kCanAutomate, Parametros::kParamTipo, 0);
	parameters.addParameter(STR16("BW"), STR16("Octs"), 0, 0.1, Vst::ParameterInfo::kCanAutomate, Parametros::kParamBW, 0);
	parameters.addParameter(STR16("Gain"), STR16("dB"), 0, 0.1, Vst::ParameterInfo::kCanAutomate, Parametros::kParamGain, 0);

	//Tchar es UTF16 char
	// u"hola" codifica en utf-16
	Vst::Parameter* param = new Vst::RangeParameter( u"Attack", 6, (Vst::TChar *) u"unit", 20., 20000., 100., 1000, Vst::ParameterInfo::kCanAutomate);
	param->setPrecision(4); // fractional sig digits|
	parameters.addParameter(param);


	return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API filtros2Controller::terminate ()
{
	// Here the Plug-in will be de-instantiated, last possibility to remove some memory!

	//---do not forget to call parent ------
	return EditControllerEx1::terminate ();
}

//------------------------------------------------------------------------
tresult PLUGIN_API filtros2Controller::setComponentState (IBStream* state)
{
	// Here you get the state of the component (Processor part)
	if (!state)
		return kResultFalse;

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API filtros2Controller::setState (IBStream* state)
{
	// Here you get the state of the controller

	/*
	IBStreamer streamer(state, kLittleEndian);
	float savedParam1 = 0.f;
	if (streamer.readFloat(savedParam1) == false)
		return kResultFalse;

	// sync with our parameter
	//todo arreglar esto
	if (auto param = parameters.getParameter(Parametros::kParamFc))
		param->setNormalized(savedParam1);
	*/


	return kResultTrue;
}

//------------------------------------------------------------------------
tresult PLUGIN_API filtros2Controller::getState (IBStream* state)
{
	// Here you are asked to deliver the state of the controller (if needed)
	// Note: the real state of your plug-in is saved in the processor

	return kResultTrue;
}

//------------------------------------------------------------------------
IPlugView* PLUGIN_API filtros2Controller::createView (FIDString name)
{
	// Here the Host wants to open your editor (if you have one)
	if (FIDStringsEqual (name, Vst::ViewType::kEditor))
	{
		// create your editor here and return a IPlugView ptr of it
        return nullptr;
	}
	return nullptr;
}

/*
tresult PLUGIN_API getParamStringByValue(Vst::ParamID tag, Vst::ParamValue valueNormalized, Vst::String128 string) SMTG_OVERRIDE {
	if (Vst::Parameter* parameter = getParameterObject(tag))
	{
		parameter->setPrecision(0); //  <-- set precision here
		parameter->toString(valueNormalized, string);
		return kResultTrue;
	}
	return kResultFalse;
}
*/


//Copiado y adaptado - convierte int a string
void intToString128(int value, Steinberg::Vst::String128 out)
{
#if SMTG_CPP_STANDARD >= 17
	std::u16string str = std::to_u16string(value);
	std::copy_n(str.c_str(), std::min<size_t>(str.size(), 127), out);
	out[std::min<size_t>(str.size(), 127)] = 0;
#else
	char temp[32];
	sprintf(temp, "%d", value);
	for (int i = 0; i < 128 && temp[i] != 0; ++i)
		out[i] = static_cast<Steinberg::Vst::TChar>(temp[i]);
	out[127] = 0;
#endif
}

//inspirada de arriba
void stringToString128(char* string, Steinberg::Vst::String128 out)
{
#if SMTG_CPP_STANDARD >= 17
	std::u16string str = std::to_u16string(value);
	std::copy_n(str.c_str(), std::min<size_t>(str.size(), 127), out);
	out[std::min<size_t>(str.size(), 127)] = 0;
#else
	for (int i = 0; i < 128 && string[i] != 0; ++i)
		out[i] = static_cast<Steinberg::Vst::TChar>(string[i]);
	out[127] = 0;
#endif
}



//String que muestra el valor del parametro
tresult PLUGIN_API filtros2Controller::getParamStringByValue(Vst::ParamID tag, Vst::ParamValue valueNormalized, Vst::String128 string) {
	

	Vst::Parameter* parameter = getParameterObject(tag);

	/*
	if (parameter)
	{
		parameter->toString(valueNormalized, string);
		return kResultTrue;
	}
	*/

	//Donde voy a guardar el string comun, en ascii, del valor que quiero mostrar, antes de pasarlo a utf-16 (formato de steinberg)
	char string_val[128];


	if (tag == kParamFc) {
		int displayValue = normalized_to_log_freq(valueNormalized); //Igualo al valor interno que tomo. TODO HACER BIEN


		sprintf((char*)string, "%d", displayValue);
		intToString128(displayValue, string);

		return kResultOk;
	}

	if (tag == kParamOrden) {
		int displayValue = normalized_to_Orden(valueNormalized, 250);


		sprintf((char*) string, "%d", displayValue);
		intToString128(displayValue, string);

		return kResultOk;
	}

	if (tag == kParamTipo) {

		int displayValue = static_cast<int>(valueNormalized * NTiposFiltro +0.5);  // Paso a 0-N. 0.5 es para el redondeo

		char* tipo;
		if (displayValue == TiposFiltro::Butterworth) tipo = "Butterworth";
		else if (displayValue == TiposFiltro::Chebyshev) tipo = "Chebyshev";
		else if (displayValue == TiposFiltro::Elliptic) tipo = "Elliptic";
		else if (displayValue == TiposFiltro::LP) tipo = "Low Pass";
		else if (displayValue == TiposFiltro::HP) tipo = "High Pass";
		else if (displayValue == TiposFiltro::Notch2) tipo = "Notch (order 2)";
		else if (displayValue == TiposFiltro::Resonator2) tipo = "Resonator (order 2)";
		else if (displayValue == TiposFiltro::AllPoleResonator2) tipo = "All Pole Resonator (order 2)";
		else if (displayValue == TiposFiltro::ParametricEq2) tipo = "Parametric Eq (order 2)";
		else if (displayValue == TiposFiltro::LowShelveEq2) tipo = "Low Shelving Eq (order 1)";
		else if (displayValue == TiposFiltro::HighShelveEq2) tipo = "High Shelving Eq (order 1)";
		else if (displayValue == TiposFiltro::NTiposFiltro) tipo = "-";

		stringToString128(tipo, string);

		return kResultOk;
	}

	if (tag == kParamBW) {
		//int displayValue = valueNormalized* 44100 / 2.; //paso del valor continuo 0-1 al Nyquist 
		//TODO hacer bien

		float displayValue = 3 * valueNormalized; //cantidad de octavas

		sprintf(string_val, "%1.2f", displayValue);
		stringToString128(string_val, string);

		return kResultOk;
	}

	if (tag == kParamGain) {

		//std::string source;
		//string = convert.from_bytes(source);


		float displayValue = normalized_to_dB(valueNormalized);
		sprintf(string_val, "%1.2f", displayValue);		
		stringToString128(string_val, string);

		return kResultOk;
	}



	return EditControllerEx1::getParamStringByValue(tag, valueNormalized, string); //Si no es un parametro asi, llamo a la fun original
}


/*
tresult PLUGIN_API filtros2Controller::setParamNormalized(ParamID tag, ParamValue value) {
	if (tag == kMyParamID) {
		// update internal state or trigger something in the UI
	}
	return EditControllerEx1::setParamNormalized(tag, value);
}
*/

//------------------------------------------------------------------------
} // namespace MyCompanyName
