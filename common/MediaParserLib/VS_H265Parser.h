
#ifndef VS_H265PARSER_H
#define VS_H265PARSER_H

#include <stdint.h>

int32_t ResolutionFromNAL_H265(const unsigned char *in, int32_t insize, int32_t& width, int32_t& height);
int32_t ResolutionFromBitstream_H265(const unsigned char *in, int32_t insize, int32_t& width, int32_t& height);
int32_t TypeSliceFromNAL_H265(unsigned char *in, int32_t insize);
int32_t TypeSliceFromBitstream_H265(unsigned char *in, int32_t insize, int32_t& irap);

#endif /* VS_H265PARSER_H */