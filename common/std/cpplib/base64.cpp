#include "base64.h"

#include <cstdint>

static constexpr uint8_t base64_encoding_table[] =
{
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
	'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
	'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', '0', '1', '2', '3',
	'4', '5', '6', '7', '8', '9', '+', '/',
};
static_assert(sizeof(base64_encoding_table) == 64, "");

static constexpr uint8_t base64_decoding_table[] =
{
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 62,  0,  0,  0, 63,
	 52, 53, 54, 55, 56, 57, 58, 59, 60, 61,  0,  0,  0,  0,  0,  0,
	  0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
	 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,  0,  0,  0,  0,  0,
	  0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
};
static_assert(sizeof(base64_decoding_table) == 256, "");

constexpr bool base64_check_tables(size_t i)
{
	return i == sizeof(base64_encoding_table) || (base64_decoding_table[base64_encoding_table[i]] == i && base64_check_tables(i + 1));
}
static_assert(base64_check_tables(0), "");

static constexpr int mod_table[] = {0, 2, 1};

bool base64_encode(const void* input, size_t input_length, char* output, size_t& output_length)
{
	auto input_data = static_cast<const uint8_t*>(input);

	const size_t required_output_length = 4 * ((input_length + 2) / 3);
	if (output == nullptr || output_length < required_output_length)
	{
		output_length = required_output_length;
		return false;
	}
	output_length = required_output_length;

	for (size_t i = 0, j = 0; i < input_length;) {

		uint32_t octet_a = i < input_length ? input_data[i++] : 0;
		uint32_t octet_b = i < input_length ? input_data[i++] : 0;
		uint32_t octet_c = i < input_length ? input_data[i++] : 0;

		uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

		output[j++] = base64_encoding_table[(triple >> 3 * 6) & 0x3F];
		output[j++] = base64_encoding_table[(triple >> 2 * 6) & 0x3F];
		output[j++] = base64_encoding_table[(triple >> 1 * 6) & 0x3F];
		output[j++] = base64_encoding_table[(triple >> 0 * 6) & 0x3F];
	}

	for (int i = 0; i < mod_table[input_length % 3]; i++)
		output[output_length - 1 - i] = '=';

	return true;
}

bool base64_decode(const char* input, size_t input_length, void* output, size_t& output_length)
{
	auto output_data = static_cast<uint8_t*>(output);

	if (input_length % 4 != 0)
	{
		output_length = 0;
		return false;
	}

	const size_t required_output_length = input_length / 4 * 3
		- (input[input_length - 1] == '=' ? 1 : 0)
		- (input[input_length - 2] == '=' ? 1 : 0);
	if (output_data == nullptr || output_length < required_output_length)
	{
		output_length = required_output_length;
		return false;
	}
	output_length = required_output_length;

	for (size_t i = 0, j = 0; i < input_length;) {
		uint32_t sextet_a = input[i] == '=' ? 0 & i++ : base64_decoding_table[static_cast<unsigned char>(input[i++])];
		uint32_t sextet_b = input[i] == '=' ? 0 & i++ : base64_decoding_table[static_cast<unsigned char>(input[i++])];
		uint32_t sextet_c = input[i] == '=' ? 0 & i++ : base64_decoding_table[static_cast<unsigned char>(input[i++])];
		uint32_t sextet_d = input[i] == '=' ? 0 & i++ : base64_decoding_table[static_cast<unsigned char>(input[i++])];

		uint32_t triple = (sextet_a << 3 * 6)
			+ (sextet_b << 2 * 6)
			+ (sextet_c << 1 * 6)
			+ (sextet_d << 0 * 6);

		if (j < output_length) output_data[j++] = (triple >> 2 * 8) & 0xFF;
		if (j < output_length) output_data[j++] = (triple >> 1 * 8) & 0xFF;
		if (j < output_length) output_data[j++] = (triple >> 0 * 8) & 0xFF;
	}

	return true;
}
