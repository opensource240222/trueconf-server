#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <memory>
#include <string.h>

void FillEmptyFrame(uint8_t* frame, size_t width, size_t height, int color);

struct VSSize
{
	uint32_t width = 0;
	uint32_t height = 0;

	inline uint32_t Square() const
	{
		return width * height;
	}

	inline bool operator==(const VSSize& other) const
	{
		return (width == other.width && height == other.height);
	}

	inline bool operator!=(const VSSize& other) const
	{
		return (width != other.width || height != other.height);
	}

	inline void operator*=(float f)
	{
		width *= f;
		height *= f;
	}

	inline VSSize operator/(unsigned int f) const
	{
		return { width / f, height / f };
	}
};

struct VSPoint
{
	int32_t x = 0;
	int32_t y = 0;

	inline VSPoint operator+(const VSPoint& other)
	{
		return { x + other.x, y + other.y };
	}

	inline bool operator==(const VSPoint& other) const
	{
		return (x == other.x && y == other.y);
	}

	inline bool operator!=(const VSPoint& other) const
	{
		return (x != other.x || y != other.y);
	}

	inline void operator*=(float f)
	{
		x *= f;
		y *= f;
	}

	inline VSPoint operator/(int f) const
	{
		return { x / f, y / f };
	}
};

struct VSRect
{
	VSPoint offset;
	VSSize size;

	VSRect() = default;

	VSRect(int32_t x, int32_t y, uint32_t width, uint32_t height) :
		offset{ x, y },
		size{ width, height }
	{};

	VSRect(VSPoint offset, VSSize size) :
		offset(offset),
		size(size)
	{};

	inline uint32_t Square()
	{
		return size.Square();
	}

	inline bool operator==(const VSRect& other) const
	{
		return (offset == other.offset && size == other.size);
	}

	inline bool operator!=(const VSRect& other) const
	{
		return (offset != other.offset || size != other.size);
	}
};

class VSPlaneView
{
public:
	VSPlaneView() = default;
	VSPlaneView(uint8_t* data, const VSSize size);
	VSPlaneView(uint8_t* data, const VSSize size, ptrdiff_t pitch);

	VSSize Size() const;
	bool IsContinuous();

	VSPlaneView GetSubview(const VSRect& area);

	void CopyTo(VSPlaneView dst);
	void Fill(uint8_t value);

	uint8_t* GetLine(uint32_t line);
	uint8_t* Data();

private:
	VSSize m_size{ 0, 0 };
	uint8_t* m_data{ nullptr };
	ptrdiff_t m_pitch{ 0 };
};

class VSI420ImageView
{
public:
	VSI420ImageView() = default;
	VSI420ImageView(uint8_t* data, VSSize size);
	VSI420ImageView(const VSPlaneView& y, const VSPlaneView& u, const VSPlaneView& v);

	VSSize Size() const;

	VSI420ImageView GetSubview(const VSRect& area);

	void CopyTo(const VSI420ImageView& dst);
	void Fill(uint32_t y);

	VSPlaneView GetPlaneY();
	VSPlaneView GetPlaneU();
	VSPlaneView GetPlaneV();

private:
	VSPlaneView planeY;
	VSPlaneView planeU;
	VSPlaneView planeV;
};
