#pragma once
class VS_Container;

char * VS_Base64EncodeAlloc(const void *in_buf, const uint32_t in_sz, uint32_t &out_sz);
void * VS_Base64DecodeAlloc(void *in_buf, const uint32_t in_sz, uint32_t &out_sz);
