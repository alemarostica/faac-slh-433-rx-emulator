#pragma once

#include "faac_slh_rx_emu_structs.h"

#define FAILED_TO_PARSE 0x0BADC0DE

void parse_faac_slh_normal(void* context, FuriString* buffer);
void parse_faac_slh_prog(void* context, FuriString* buffer);
