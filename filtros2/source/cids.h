//------------------------------------------------------------------------
// Copyright(c) 2025 ASKA.
//------------------------------------------------------------------------

#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"

namespace MyCompanyName {
//------------------------------------------------------------------------
static const Steinberg::FUID kfiltros2ProcessorUID (0xAA607D5B, 0x5E305746, 0x81352EFD, 0x76E64261);
static const Steinberg::FUID kfiltros2ControllerUID (0x94ED2D59, 0x4A795A4E, 0xB27412BE, 0x01853038);

#define filtros2VST3Category "Fx"

//------------------------------------------------------------------------
} // namespace MyCompanyName

enum Parametros : Steinberg::Vst::ParamID
{
    kParamFc = 102, // should be a unique id...
    kParamb0,
    kParama1, 
    kParamOrden,
    kParamBW,
    kParamGain,
    kParamTipo

};

enum TiposFiltro {

    Butterworth = 0,
    Chebyshev,
    Elliptic,
    LP,
    HP,
    Notch2,
    Resonator2,
    AllPoleResonator2,
    ParametricEq2,
    LowShelveEq2,
    HighShelveEq2,
    NTiposFiltro

};
//std::vector<char*> TiposFiltroNombres({ "Butterworth", "Chebyshev", "Elliptic", "Notch (orden2)", "Resonator (orden2)", "Parametric Eq (orden 2)" });
