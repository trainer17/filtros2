//------------------------------------------------------------------------
// Copyright(c) 2025 ASKA.
//------------------------------------------------------------------------

#pragma once

#include "public.sdk/source/vst/vstaudioeffect.h"
#include "circularBuff.hpp"
#include "filtros.hpp"

namespace MyCompanyName {

//------------------------------------------------------------------------
//  filtros2Processor
//------------------------------------------------------------------------
class filtros2Processor : public Steinberg::Vst::AudioEffect
{
public:
	filtros2Processor ();
	~filtros2Processor () SMTG_OVERRIDE;

    // Create function
	static Steinberg::FUnknown* createInstance (void* /*context*/) 
	{ 
		return (Steinberg::Vst::IAudioProcessor*)new filtros2Processor; 
	}

	//--- ---------------------------------------------------------------------
	// AudioEffect overrides:
	//--- ---------------------------------------------------------------------
	/** Called at first after constructor */
	Steinberg::tresult PLUGIN_API initialize (Steinberg::FUnknown* context) SMTG_OVERRIDE;
	
	/** Called at the end before destructor */
	Steinberg::tresult PLUGIN_API terminate () SMTG_OVERRIDE;
	
	/** Switch the Plug-in on/off */
	Steinberg::tresult PLUGIN_API setActive (Steinberg::TBool state) SMTG_OVERRIDE;

	/** Will be called before any process call */
	Steinberg::tresult PLUGIN_API setupProcessing (Steinberg::Vst::ProcessSetup& newSetup) SMTG_OVERRIDE;
	
	/** Asks if a given sample size is supported see SymbolicSampleSizes. */
	Steinberg::tresult PLUGIN_API canProcessSampleSize (Steinberg::int32 symbolicSampleSize) SMTG_OVERRIDE;

	/** Here we go...the process call */
	Steinberg::tresult PLUGIN_API process (Steinberg::Vst::ProcessData& data) SMTG_OVERRIDE;
		
	/** For persistence */
	Steinberg::tresult PLUGIN_API setState (Steinberg::IBStream* state) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API getState (Steinberg::IBStream* state) SMTG_OVERRIDE;

//------------------------------------------------------------------------

	//Se lo agrego yo
	void filtros2Processor::processInputParameterChanges(Steinberg::Vst::ProcessData& data);
	void filtros2Processor::processInputEvents(Steinberg::Vst::ProcessData& data); //same
	void filtros2Processor::processInputAudio(Steinberg::Vst::ProcessData& data); //same
	void filtros2Processor::clearOutputBuses(Steinberg::Vst::ProcessData& data, int bus_from, int bus_to);
	void filtros2Processor::clearOutputChannels(Steinberg::Vst::ProcessData& data, int bus, int chann_from, int chann_to);
	void filtros2Processor::processMio(int nsamps, int channel, Steinberg::Vst::Sample32* channin, Steinberg::Vst::Sample32* channout);



	//std::unique_ptr<Steinberg::Vst::DataExchangeHandler> dataExchange;

//------------------------------------------------------------------------
protected:

	//Parametros fijos de implementacion
	const int n_channs = 2; //Plugin stereo
	const int ordenMax = 250; //Maximo orden para los filtros . Si cambio aca, tengo que cambiar el controller .cpp tambien, donde agrego este parametro
	const int bufsize = 4096; //tamaño de los buffers internos para procesar el audio. Solo necesito "ordenMax", pero como el Daw me puede pedir con bloques de esa cantidad de samples...

	
	//Parametros variables.
	//todo no arrancan con los valores de los sliders, sino los que defino acá
	int tipoFiltroId = TiposFiltro::Butterworth ; //Tipo de filtro actual. Ver cids.h

	Steinberg::Vst::ParamValue fc = 500.; //frec de corte
	Steinberg::Vst::ParamValue bw = 100.; //Bandwith del filtro en HZ. Usado cuando aplica
	Steinberg::Vst::ParamValue bw_slider = 0.3 ; //Valor del slider de este param
	//Steinberg::Vst::ParamValue Gain_dB = 0.;  //Gain del Parametric eq, shelving eq en dB. No lo uso
	Steinberg::Vst::ParamValue Gain = 1.; //Gain lineal de parametric y shelving eqs

	int orden = 1; //Orden del filtro. Usado cuando aplica



	//Miembros necesarios para la implementacion
	Filtro<float> filt;
	std::vector<circularBuffer<float> > inputs_buff;  //meimoizacion de la entrada en cada canal
	std::vector<circularBuffer<float> > outputs_buff;
	
	int debugCounter = 0;


};

//------------------------------------------------------------------------
} // namespace MyCompanyName
