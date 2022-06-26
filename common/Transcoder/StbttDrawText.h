#pragma once

#include "std-generic/cpplib/string_view.h"

#include <vector>
#include <cinttypes>

std::vector<uint8_t> StbttDrawText(string_view text, uint32_t w, uint32_t h);

void StbttDrawText(string_view text, uint32_t textHeight, uint8_t* image, uint32_t imgWidth, uint32_t imgHeight);
