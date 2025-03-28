#pragma once

#include "faac_slh_rx_emu_structs.h"

#define FAILED_TO_PARSE 0x0BADC0DE

void decode_faac_slh(void* context, FaacSLHRxEmuModel* model, FuriString* buffer);

FaacSLHData* faac_slh_data_alloc();
void faac_slh_data_free(FaacSLHData* data);
