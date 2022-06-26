#pragma once

#include <vector>
#include <string>
#include <algorithm>
#include "VSVideoUtils.h"

enum EDisplayNamePosition : uint8_t
{
	DNP_NONE,
	DNP_TOP,
	DNP_BOTTOM
};

class VS_TextDrawer
{
public:
	VS_TextDrawer(
		VSSize image, VSRect drawed,
		EDisplayNamePosition position,
		const std::string& text);

	void SetDisplayNamePosition(EDisplayNamePosition position);

	void DrawNameText(uint8_t* image, uint8_t color);

private:
	int m_txtH = 0;
	std::vector<uint8_t> m_textAlpha;
	std::vector<uint8_t> m_textOneMinusAlpha;
	EDisplayNamePosition m_displayNamePosition = DNP_TOP;

	VSSize m_image;
	VSRect m_drawed;
};
