#pragma once

#include "faac_slh_rx_emu_structs.h"

#define FAILED_TO_PARSE 0x0BADC0DE

void decode_faac_slh(FaacSLHRxEmuModel* model, FuriString* buffer);
