#pragma once

#include "faac_slh_rx_emu_structs.h"
#include <furi.h>

#ifdef TAG
#undef TAG
#endif
#define TAG "FaacSLHRxEmuUtils"

/**
 * @brief   Extract a FuriString from another based on start index, a needle and a delimiter character.
 * @details This function extracts a subset of a FuriString given a start index, a needle after which to extract in the original FuriString and a delimiter character to stop at.
 * @param   buffer  Pointer to the FuriString to extract from.
 * @param   start_index the starting index in the first FuriString.
 * @param   text    cstr, needle after which to extract the substring.
 * @param   delim   character to stop at (excluded).
 * @param   result  Pointer to the destination FuriString.
*/
size_t __furi_string_extract_string(
    FuriString* buffer,
    size_t start_index,
    char* text,
    char delim,
    FuriString* result);

/**
 * @brief   Extract a FuriString from another based on start index, a needle and a delimiter character, will stop at end of FuriString.
 * @details This function extracts a subset of a FuriString given a start index, a needle after which to extract in the original FuriString and a delimiter character to stop at. This function will stop at the end of a FuriString if the delimiter character is not found.
 * @param   buffer  Pointer to the FuriString to extract from.
 * @param   start_index the starting index in the first FuriString.
 * @param   text    cstr, needle after which to extract the substring.
 * @param   delim   character to stop at (excluded).
 * @param   result  Pointer to the destination FuriString.
*/
size_t __furi_string_extract_string_until(
    FuriString* buffer,
    size_t start_index,
    char* text,
    char until_delim,
    FuriString* result);

/**
 * @brief   Extract a uint32_t from a FuriString based on a needle and a delimiter character, will stop at end of FuriString.
 * @details This function extracts a hexadecimal uint32_t from a FuriString given a needle after which to extract in the original FuriString and a delimiter character to stop at. This function will stop at the end of a FuriString if the delimiter character is not found.
 * @param   buffer  Pointer to the FuriString to extract from.
 * @param   text    cstr, needle after which to extract the integer.
 * @param   delim   character to stop at (excluded).
 * @param   default_value   Default value to return if the extraction did not yield anything valid.
 * @return  The extracted integer
*/
uint32_t
    __furi_string_extract_int(FuriString* buffer, char* text, char delim, uint32_t default_value);

/**
 * @brief   Convert hex FuriString into uint32_t
 * @details This function takes a FuriString with a hexadecimal value and converts it into a uint32_t
 * @param   str Pointer to the FuriString to convert
 * @return  The converted integer
*/
uint32_t __furi_string_hex_to_uint32(FuriString* str);

/**
 * @brief   Redraw the GUI
 * @details This function redraws the screen
*/
void __gui_redraw();
