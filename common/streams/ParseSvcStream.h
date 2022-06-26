#pragma once

#include "Protocol.h"
#include <cstring>

unsigned char* ParseSvcStream(unsigned char *in, int size, bool svc, int *lsize, stream::SVCHeader *h, int *shift_size);