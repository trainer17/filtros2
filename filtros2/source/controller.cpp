//------------------------------------------------------------------------
// Copyright(c) 2025 ASKA.
//------------------------------------------------------------------------

#include "controller.h"
#include "cids.h"
#include "base/source/fstreamer.h"


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
	// Here you could register some parameters
	parameters.addParameter(STR16("Fc"), STR16("Hz"), 0, .5, Vst::ParameterInfo::kCanAutomate, Parametros::kParamFc, 0);
	//A VST 3 parameter is always normalized (its value is a floating point value between [0.0, 1.0]), here its default value is set to 0.5.

	parameters.addParameter(STR16("b0"), STR16(""), 0, .5, Vst::ParameterInfo::kCanAutomate, Parametros::kParamb0, 0);
	parameters.addParameter(STR16("a1"), STR16(""), 0, .5, Vst::ParameterInfo::kCanAutomate, Parametros::kParama1, 0);
	parameters.addParameter(STR16("Orden"), STR16(""), 250, 0.01, Vst::ParameterInfo::kCanAutomate, Parametros::kParamOrden, 0);
	parameters.addParameter(STR16("Tipo"), STR16(""), NTiposFiltro, 0, Vst::ParameterInfo::kCanAutomate, Parametros::kParamTipo, 0);
	parameters.addParameter(STR16("BW"), STR16("Hz"), 0, 0.1, Vst::ParameterInfo::kCanAutomate, Parametros::kParamBW, 0);


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
	
	if (tag == kParamFc) {
		int displayValue = valueNormalized * 44100 / 2.; //Igualo al valor interno que tomo. TODO HACER BIEN


		sprintf((char*)string, "%d", displayValue);
		intToString128(displayValue, string);

		return kResultOk;
	}

	if (tag == kParamOrden) {
		int displayValue = 1 + static_cast<int>(valueNormalized * 249.999);  // Ensures 1–250


		sprintf((char*) string, "%d", displayValue);
		intToString128(displayValue, string);

		return kResultOk;
	}

	if (tag == kParamTipo) {

		int displayValue = static_cast<int>(valueNormalized * NTiposFiltro +0.5);  // Paso a 0-N

		char* tipo;
		//tipo = TiposFiltroNombres[displayValue]; no se por que eso no anda
		if (displayValue == 0) tipo = "Butterworth";
		if (displayValue == 1) tipo = "Chebyshev";
		if (displayValue == 2) tipo = "Elliptic";
		if (displayValue == 3) tipo = "Notch (order 2)";
		if (displayValue == 4) tipo = "Resonator (order 2)";
		if (displayValue == 5) tipo = "Parametric Eq (order 2)";
		if (displayValue == 6) tipo = "-";
	

		stringToString128(tipo, string);

		return kResultOk;
	}

	if (tag == kParamBW) {
		int displayValue = valueNormalized* 44100 / 2.; //paso del valor continuo 0-1 al Nyquist 
		//TODO hacer bien

		sprintf((char*)string, "%d", displayValue);
		intToString128(displayValue, string);

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
