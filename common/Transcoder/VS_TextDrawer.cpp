#include "StbttDrawText.h"
#include "VS_TextDrawer.h"
#include "IppLib2/VSVideoProcessingIpp.h"

#include <cstring>

VS_TextDrawer::VS_TextDrawer(
	VSSize image, VSRect drawed,
	EDisplayNamePosition position,
	const std::string& text)
	: m_image(image)
	, m_drawed(drawed)
	, m_displayNamePosition(position)
{
	m_txtH = (std::max(m_drawed.size.height / 16, uint32_t(14)) + 1) &~1;
	m_textAlpha = StbttDrawText(text, m_drawed.size.width, m_txtH);
	m_textOneMinusAlpha = m_textAlpha;

	for (auto& a : m_textOneMinusAlpha)
		a = (255 - float(a)) * 3 / 4;

	for (auto& a : m_textAlpha)
		a = float(a) / 255.f * 240;
}

void VS_TextDrawer::SetDisplayNamePosition(EDisplayNamePosition position)
{
	m_displayNamePosition = position;
}

void VS_TextDrawer::DrawNameText(uint8_t* image, uint8_t color)
{
	if (m_txtH && m_displayNamePosition != DNP_NONE)
	{
		uint8_t* alpha = m_textAlpha.data();
		uint32_t textOffset = 0;

		if (m_displayNamePosition == DNP_BOTTOM)
			textOffset = m_drawed.size.height - m_txtH;

		uint8_t *mem = image + (m_drawed.offset.y + textOffset) * m_image.width + m_drawed.offset.x;

		ippiMul_8u_C1IRSfs(
			m_textOneMinusAlpha.data(), m_drawed.size.width,
			mem, m_image.width,
			{ (int)m_drawed.size.width, m_txtH },
			8);

		ippiAdd_8u_C1IRSfs(
			m_textAlpha.data(), m_drawed.size.width,
			mem, m_image.width,
			{ (int)m_drawed.size.width, m_txtH },
			0);
	}
}
