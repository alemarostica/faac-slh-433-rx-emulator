#pragma once

#include "faac_slh_rx_emu_structs.h"

#define FAILED_TO_PARSE 0x0BADC0DE

/**
 * @brief   Parse a normal FAAC SLH key.
 * @details This function parses a normal key into the model_normal of the app.
 * @param   context Pointer to FaacSLHRxEmuApp.
 * @param   buffer  Pointer to FuriString containing a decoded key.
*/
void parse_faac_slh_normal(void* context, FuriString* buffer);

/**
 * @brief   Parse a FAAC SLH Master Remote Prog Key.
 * @details This function parses a prog key into the model_prog of the app.
 * @param   context Pointer to FaacSLHRxEmuApp.
 * @param   buffer  Pointer to FuriString containing a decoded key.
*/
void parse_faac_slh_prog(void* context, FuriString* buffer);
