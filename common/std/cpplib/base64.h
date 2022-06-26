#pragma once

#include <cstddef>

// Encodes raw data (input, input_length) in base64 and stores in buffer (output, output_length).
// If buffer isn't provided (output == null) or isn't big enough to hold the
// result, sets output_length to required buffer size and returns false.
// On success sets output_length to size of the result and returns true.
bool base64_encode(const void* input, size_t input_length, char* output, size_t& output_length);

// Decodes base64-encooded string (input, input_length) and stores in buffer (output, output_length).
// If buffer isn't provided (output == null) or isn't big enough to hold the
// result, sets output_length to required buffer size and returns false.
// On success sets output_length to size of the result and returns true.
bool base64_decode(const char* input, size_t input_length, void* output, size_t& output_length);
