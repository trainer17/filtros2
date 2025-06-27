//------------------------------------------------------------------------
// Copyright(c) 2025 ASKA.
//------------------------------------------------------------------------

#include "processor.h"
#include "cids.h"
#include "funciones.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h" // Para parametros en el daw
#include "pluginterfaces/vst/ivstevents.h" // para eventos midi
#include "circularBuff.hpp" //circular buffers 
#include "filtros.hpp" //mi codigo de filtros
#include "public.sdk/source/vst/utility/dataexchange.h" //send data from the processor to the controller.
#include "public.sdk/source/vst/vstaudioprocessoralgo.h"

using namespace Steinberg;

namespace MyCompanyName {
	//------------------------------------------------------------------------
	// filtros2Processor
	//------------------------------------------------------------------------
	filtros2Processor::filtros2Processor()
	{
		//--- set the wanted controller for our processor
		setControllerClass(kfiltros2ControllerUID);
	}

	//------------------------------------------------------------------------
	filtros2Processor::~filtros2Processor()
	{
	}

	//------------------------------------------------------------------------
	tresult PLUGIN_API filtros2Processor::initialize(FUnknown* context)
	{
		// Here the Plug-in will be instantiated

		//---always initialize the parent-------
		tresult result = AudioEffect::initialize(context);
		// if everything Ok, continue
		if (result != kResultOk)
		{
			return result;
		}

		//--- create Audio IO ------
		addAudioInput(STR16("Stereo In"), Steinberg::Vst::SpeakerArr::kStereo);
		addAudioOutput(STR16("Stereo Out"), Steinberg::Vst::SpeakerArr::kStereo);

		/* If you don't need an event bus, you can remove the next line */
		addEventInput(STR16("Event In"), 1);


		//Creo los inputs y outputs buffers


		for (int chann = 0; chann < n_channs; chann++) {
			inputs_buff.emplace_back(bufsize);
			outputs_buff.emplace_back(bufsize);
		}

		/* Inicializo el filtro */
		//filt.as = std::vector<float>({ 1, 0 }); //recursivos
		//filt.bs = std::vector<float>({ 0.5, 0 }); //lineales

		return kResultOk;
	}

	//------------------------------------------------------------------------
	tresult PLUGIN_API filtros2Processor::terminate()
	{
		// Here the Plug-in will be de-instantiated, last possibility to remove some memory!

		/*delete[] inputs_buff[0]; delete[] inputs_buff[1];
		delete[] inputs_buff;

		delete[] outputs_buff[0]; delete[] outputs_buff[1];
		delete[] outputs_buff;
		*/
		//---do not forget to call parent ------
		return AudioEffect::terminate();
	}

	//------------------------------------------------------------------------
	tresult PLUGIN_API filtros2Processor::setActive(TBool state)
	{
		//--- called when the Plug-in is enable/disable (On/Off) -----
		return AudioEffect::setActive(state);

		//Todo: borrar buffer de in y out
	}

	//Devuelve un sample random entre -1 y 1
#include <cstdlib>  // para std::rand
	Vst::Sample32 randomSample() {
		return -1.0f + static_cast<float>(std::rand()) / RAND_MAX * 2.0f;
	}

	//------------------------------------------------------------------------
	tresult PLUGIN_API filtros2Processor::process(Vst::ProcessData& data)
	{
		//-- Flush case: we only need to update parameter, noprocessing possible
		if (data.numInputs == 0 || data.numSamples == 0)
			return kResultOk;

		//--- First : Read inputs parameter changes-----------
		processInputParameterChanges(data);




		//---Second: Read input events------------- (Ej MIDI)
		processInputEvents(data);

		//---Tercero: Procesar audio-------------
		if (data.numSamples > 0) processInputAudio(data);

		return kResultOk;
	}

	void filtros2Processor::processInputAudio(Vst::ProcessData& data) {



		//--- ------------------------------------------
		// here as example a default implementation where we try to copy the inputs to the outputs:
		// if less input than outputs then clear outputs
		//--- ------------------------------------------

		int32 bus = 0;

		int32 numChannels = data.inputs[bus].numChannels;

		//---get audio buffers using helper-functions(vstaudioprocessoralgo.h)-------------
		//uint32 sampleFramesSize = getSampleFramesSizeInBytes(processSetup, data.numSamples);

			for (int32 chann = 0; chann < std::min(numChannels, n_channs); chann++)
			{

				//void** in = getChannelBuffersPointer(processSetup, data.inputs[bus]);
				//void** out = getChannelBuffersPointer(processSetup, data.outputs[bus]);

				Vst::Sample32* channin = data.inputs[bus].channelBuffers32[chann];
				Vst::Sample32* channout = data.outputs[bus].channelBuffers32[chann];

				if (!channin || !channout) return; //Si los punteros son NULL no hago nada

				processMio(data.numSamples, chann, channin, channout); 
			}

		// Here could check the silent flags
		// now we will produce the output
		// mark our outputs has not silent
		data.outputs[bus].silenceFlags = 0;

		return;

	}

	//Acá hago lo de verdad interesante del plugin. Recibo audio y lo proceso
	void filtros2Processor::processMio(int nsamps, int channel, Vst::Sample32* channin, Vst::Sample32* channout) {

		/*debugCounter++;
		if (debugCounter %  2 == 0) {
			channin[0] = 1.;
		}
		*/



		// load input samples en memoria
		//No avanza los indices de sample actual (agrega "mas adelante")
		inputs_buff[channel].copyfromIn(channin, nsamps); //Copio el bloque de entrada a mi buffer interno

		//Proceso - Filtro n muestras
		//Esto encola n nuevos samples filtrados en el outputbuffer, pero no avanza el indice actual (los agrega "mas adelante").
		//Tampoco avanza el indice de lectura
		filt.filter(inputs_buff[channel], outputs_buff[channel], nsamps);

		//Copio el out calculado al bloque de salida
		//Luego de esto sí avanzo los indices de los dos buffers
		outputs_buff[channel].copytoOut(channout, nsamps);

		outputs_buff[channel].advance(nsamps);
		inputs_buff[channel].advance(nsamps);

	}

	//Pone en 0 todos los buses desde from hasta to
	void filtros2Processor::clearOutputBuses(Steinberg::Vst::ProcessData& data, int bus_from, int bus_to) {

		for (int bus = bus_from; bus < bus_to; bus++) {
			data.outputs[bus].silenceFlags = data.inputs[bus].silenceFlags;
			clearOutputChannels(data, bus, 0, data.outputs[bus].numChannels);
		}
		return;
	}

	void filtros2Processor::clearOutputChannels(Steinberg::Vst::ProcessData& data, int bus, int chann_from, int chann_to) {

		//uint32 sampleFramesSize = getSampleFramesSizeInBytes(processSetup, data.numSamples);


		// clear output buffers
		for (int32 chann = chann_from; chann < chann_to; chann++)
		{
			memset(data.outputs[bus].channelBuffers32[chann], 0,
				data.numSamples * sizeof(Vst::Sample32)); //Todo ojo con tamaño del sample, si 32 o 64bits
		}
		// inform the host that this bus is silent
		data.outputs[bus].silenceFlags = ((uint64)1 << data.outputs[bus].numChannels) - 1;
		return;
	}

	//------------------------------------------------------------------------
	tresult PLUGIN_API filtros2Processor::setupProcessing(Vst::ProcessSetup& newSetup)
	{
		//--- called before any processing ----
		return AudioEffect::setupProcessing(newSetup);
	}

	//------------------------------------------------------------------------
	tresult PLUGIN_API filtros2Processor::canProcessSampleSize(int32 symbolicSampleSize)
	{
		// by default kSample32 is supported
		if (symbolicSampleSize == Vst::kSample32)
			return kResultTrue;

		// disable the following comment if your processing support kSample64
		/* if (symbolicSampleSize == Vst::kSample64)
			return kResultTrue; */

		return kResultFalse;
	}

	//------------------------------------------------------------------------
	tresult PLUGIN_API filtros2Processor::setState(IBStream* state)
	{
		// called when we load a preset, the model has to be reloaded
		IBStreamer streamer(state, kLittleEndian);

		return kResultOk;
	}

	//------------------------------------------------------------------------
	tresult PLUGIN_API filtros2Processor::getState(IBStream* state)
	{
		// here we need to save the model
		IBStreamer streamer(state, kLittleEndian);

		return kResultOk;
	}

	//------------------------------------------------------------------------



	

	void filtros2Processor::processInputParameterChanges(Vst::ProcessData& data)
	{

		double sr = processSetup.sampleRate;

		//data.inputParameterChanges can include more than 1 change for the same parameter inside a processing audio block. 
		//Here we take only the last change in the list
		if (data.inputParameterChanges)
		{
			int32 numParamsChanged = data.inputParameterChanges->getParameterCount();
			for (int32 index = 0; index < numParamsChanged; index++)
			{

				//Vst::IParamValueQueue* paramQueue = data.inputParameterChanges->getParameterData(index);
				if (auto* paramQueue = data.inputParameterChanges->getParameterData(index))
				{

					// for this parameter we could iterate the list of value changes (could be 1 per audio block or more!)
					// in this example, we get only the last value (getPointCount - 1)
					Vst::ParamValue value;
					int32 sampleOffset;
					int32 numPoints = paramQueue->getPointCount();

					switch (paramQueue->getParameterId())
					{
					case Parametros::kParamFc:
						if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue)
						{
							fc = normalized_to_log_freq(value); //Paso de 0-1 a  20hz - 20khz de manera exponencial
							// Nota, si cambio acá como mapeo fc, tambien cambiar como lo muestro en controller.cpp
							if (fc > sr / 2) fc = sr / 2 - 10;
							//fc = (value*0.9999)*sr /2. ; //Paso a 0Hz - Nyquist Hz
							//gain = pow(10., 2 * (value - 0.5));

							bw = normalized_octs_to_hz_bw(bw_slider, fc, sr);
						}
						break;

					case Parametros::kParamOrden:
						//https://steinbergmedia.github.io/vst3_dev_portal/pages/Technical+Documentation/Parameters+Automation/Index.html
						if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue)
							orden = normalized_to_Orden(value, ordenMax); //paso del valor continuo 0-1 al discreto 1-OrdenMax
						break;


					case Parametros::kParamTipo:
						//https://steinbergmedia.github.io/vst3_dev_portal/pages/Technical+Documentation/Parameters+Automation/Index.html
						if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue)
							tipoFiltroId = std::min((double)NTiposFiltro + 1, value * (NTiposFiltro + 1)); //paso del valor continuo 0-1 al 
						break;


					case Parametros::kParamBW:
						if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue)
						{
							//bw = normalized_to_log_freq(value);
							//bw = value*sr / 2.; //paso del valor continuo 0-1 al Nyquist (HZ)

							//El parametro 0-1 controla una cantidad de intervalos segun fc. de 0 a 3 octavas
							// Acá lo convierto a su valor en Hz
							// Que sea  control en octavas es una heuristica aproximada... tendria que ser asimetrica en ese caso. Aca no lo hago asi
							bw = normalized_octs_to_hz_bw(value, fc, sr);
							bw_slider = value;
						}
						break;

					case Parametros::kParamGain:
						if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue) {
							float Gain_dB = normalized_to_dB(value);
							Gain = db_to_linear(Gain_dB);
						}
						break;

					}

				}
				if (numParamsChanged > 0) filt.calcularCoeffs(tipoFiltroId, fc, sr, bw, orden, Gain);
			}


		}

		return;
	}

	void filtros2Processor::processInputEvents(Vst::ProcessData& data) {

		// get the list of all event changes
		Vst::IEventList* eventList = data.inputEvents;
		if (eventList)
		{
			int32 numEvent = eventList->getEventCount();
			for (int32 i = 0; i < numEvent; i++)
			{
				Vst::Event event;
				if (eventList->getEvent(i, event) == kResultOk)
				{
					float vel;
					// here we do not take care of the channel info of the event
					switch (event.type)
					{
						//----------------------
					case Vst::Event::kNoteOnEvent:
						// use the velocity as gain modifier: a velocity max (1) will lead to silent audio
						vel = event.noteOn.velocity; // value between 0 and 1
						break;

						//----------------------
					case Vst::Event::kNoteOffEvent:
						// noteOff reset the gain modifier
						vel = 0.f;
						break;
					}
				}
			}
		}
		return;
	}

} // namespace MyCompanyName



