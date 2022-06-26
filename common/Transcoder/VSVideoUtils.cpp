#include "VSVideoUtils.h"

void FillEmptyFrame(uint8_t* frame, size_t width, size_t height, int color)
{
	for (size_t i = 0; i < height; i++) {
		memset(frame, color, width);
		frame += width;
	}
	for (size_t i = 0; i < height / 2; i++) {
		memset(frame, 0x80, width / 2);
		frame += width / 2;
	}
	for (size_t i = 0; i < height / 2; i++) {
		memset(frame, 0x80, width / 2);
		frame += width / 2;
	}
}

VSPlaneView::VSPlaneView(uint8_t* data, const VSSize size)
	: m_size(size)
	, m_data(data)
	, m_pitch(size.width)
{
}

VSPlaneView::VSPlaneView(uint8_t* data, const VSSize size, ptrdiff_t pitch)
	: m_size(size)
	, m_data(data)
	, m_pitch(pitch)
{
}

VSSize VSPlaneView::Size() const
{
	return m_size;
}

bool VSPlaneView::IsContinuous()
{
	return m_size.width == m_pitch;
}

VSPlaneView VSPlaneView::GetSubview(const VSRect& area)
{
	uint8_t* data = m_data + area.offset.y * m_pitch + area.offset.x;
	return VSPlaneView(data, area.size, m_pitch);
}

void VSPlaneView::CopyTo(VSPlaneView dst)
{
	uint32_t height = std::min(m_size.height, dst.m_size.height);
	uint32_t width = std::min(m_size.width, dst.m_size.width);

	for (uint32_t h = 0; h < height; h++)
		std::memcpy(dst.m_data + h * dst.m_pitch, m_data + h * m_pitch, width);
}

void VSPlaneView::Fill(uint8_t value)
{
	if (IsContinuous())
	{
		std::memset(m_data, value, m_size.Square());
	}
	else
	{
		for (uint32_t h = 0; h < m_size.height; h++)
			std::memset(GetLine(h), value, m_size.width);
	}
}

uint8_t* VSPlaneView::GetLine(uint32_t line)
{
	return m_data + line * m_pitch;
}

uint8_t* VSPlaneView::Data()
{
	return GetLine(0);
}

VSI420ImageView::VSI420ImageView(uint8_t* data, VSSize size)
	: planeY(data, size)
	, planeU(planeY.Data() + planeY.Size().Square(), size / 2)
	, planeV(planeU.Data() + planeU.Size().Square(), size / 2)
{}

VSI420ImageView::VSI420ImageView(const VSPlaneView& y, const VSPlaneView& u, const VSPlaneView& v)
	: planeY(y)
	, planeU(u)
	, planeV(v)
{}

VSSize VSI420ImageView::Size() const
{
	return planeY.Size();
}

VSI420ImageView VSI420ImageView::GetSubview(const VSRect& area)
{
	return {
		planeY.GetSubview(area),
		planeU.GetSubview({area.offset / 2, area.size / 2}),
		planeV.GetSubview({area.offset / 2, area.size / 2})
	};
}

void VSI420ImageView::CopyTo(const VSI420ImageView& dst)
{
	planeY.CopyTo(dst.planeY);
	planeU.CopyTo(dst.planeU);
	planeV.CopyTo(dst.planeV);
}

void VSI420ImageView::Fill(uint32_t y)
{
	planeY.Fill(y);
	planeU.Fill(0x80);
	planeV.Fill(0x80);
}

VSPlaneView VSI420ImageView::GetPlaneY()
{
	return planeY;
}

VSPlaneView VSI420ImageView::GetPlaneU()
{
	return planeU;
}

VSPlaneView VSI420ImageView::GetPlaneV()
{
	return planeV;
}
