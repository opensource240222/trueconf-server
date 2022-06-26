#pragma once
#include "VS_ConfControlInterface.h"
#include "VS_TransmitFrameInterface.h"
class VS_CircuitStreamRelayInterface :	public VS_ConfControlInterface,
										public VS_TransmitFrameInterface
{
};