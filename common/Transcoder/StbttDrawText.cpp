#include "StbttDrawText.h"

#include <cmath>
#include <cstring>
#include "extlibs/stb/stb_truetype.h"
#include "std-generic/cpplib/utf8.h"
#include "Transcoder/stb/default_ttf.h"
#include "Transcoder/stb/seguiemj_ttf.h"
#include "VSVideoUtils.h"

struct GlyphInfo
{
	int advance_w = 0;
	int lsb = 0;
	int xmin = 0;
	int xmax = 0;
	int ymin = 0;
	int ymax = 0;
	int fontIdx = 0;
};

typedef std::vector<std::pair<char32_t /* code */, GlyphInfo>> tGlyphsInfo;

struct Font
{
	stbtt_fontinfo fontInfo;
	float scale;
	int baseline;
	int descent;
	int lineGap;
};

bool InitFont(const uint8_t* data, uint32_t height, Font& font)
{
	if (!stbtt_InitFont(&font.fontInfo, data, stbtt_GetFontOffsetForIndex(data, 0)))
		return false;

	int ascent = 0;
	int descent = 0;
	int lineGap = 0;

	stbtt_GetFontVMetrics(&font.fontInfo, &ascent, &descent, &lineGap);

	font.scale = stbtt_ScaleForPixelHeight(&font.fontInfo, height);
	font.baseline = static_cast<int>(font.scale * ascent);
	font.descent = static_cast<int>(font.scale * descent);
	font.lineGap = static_cast<int>(font.scale * lineGap);

	return true;
}

void FormGlyphsText(
	string_view text,
	const std::vector<Font> fonts,
	const int32_t maxWidth,
	tGlyphsInfo &glyphs,
	int32_t &imgWidth,
	int32_t &cropWidth)
{
	glyphs.clear();
	imgWidth = 0;
	cropWidth = 0;
	std::u32string wtext = vs::UTF8ToUTF32Convert(text);

	for (char32_t code : wtext)
	{
		GlyphInfo gi;

		for (int i = 0; i < fonts.size(); i++)
		{
			if (stbtt_FindGlyphIndex(&fonts[i].fontInfo, code))
			{
				gi.fontIdx = i;
				break;
			}
		}

		const stbtt_fontinfo* fontInfo = &fonts[gi.fontIdx].fontInfo;
		float scale = fonts[gi.fontIdx].scale;

		int lsb(0), advanceWidth(0);
		stbtt_GetCodepointHMetrics(fontInfo, int(code), &advanceWidth, &lsb);
		gi.advance_w = static_cast<int>(std::round(scale * advanceWidth));
		gi.lsb = static_cast<int>(std::round(scale * lsb));
		stbtt_GetCodepointBitmapBox(fontInfo, int(code), scale, scale, &gi.xmin, &gi.ymin, &gi.xmax, &gi.ymax);
		imgWidth += gi.advance_w;

		if (imgWidth <= maxWidth)
		{
			cropWidth = imgWidth;
			glyphs.emplace_back(int(code), gi);
		}
	}
}

std::vector<uint8_t> StbttDrawText(string_view text, uint32_t w, uint32_t h)
{
	if (w * h == 0)
		return std::vector<uint8_t>();

	std::vector<Font> fonts(2);

	if (!InitFont(default_ttf, h, fonts[0]))
		return std::vector<uint8_t>(w * h);

	if (!InitFont(seguiemj_ttf, h, fonts[1]))
		return std::vector<uint8_t>(w * h);

	std::string dots("...");
	int dotsWidth(0), textWidth(0), crop_w(0);
	tGlyphsInfo dotsGlyphs, textGlyphs;
	FormGlyphsText(dots, fonts, w, dotsGlyphs, dotsWidth, crop_w);

	if (w < dotsWidth)
		return std::vector<uint8_t>(w * h);

	FormGlyphsText(text, fonts, w - dotsWidth, textGlyphs, textWidth, crop_w);
	if (textWidth != crop_w) {
		for (const auto &it : dotsGlyphs) {
			textGlyphs.push_back(it);
		}
		textWidth = crop_w + dotsWidth;
	}

	if (w < textWidth)
		return std::vector<uint8_t>();

	std::vector<uint8_t> buffer(w * h);
	if (!textGlyphs.empty()) {
		int32_t offset_x = (w - textWidth) / 2;
		auto ptr = buffer.data() + offset_x;
		for (const auto &gi : textGlyphs) {
			int wg(0), hg(0);
			Font& font = fonts[gi.second.fontIdx];
			auto bitmap = stbtt_GetCodepointBitmap(&font.fontInfo, font.scale, font.scale, gi.first, &wg, &hg, 0, 0);
			for (int i = 0; i < hg; i++)
			{
				int offsetY = w * (i + font.baseline + gi.second.ymin);

				if (offsetY >= 0)
					memcpy(ptr + gi.second.lsb + offsetY, bitmap + i * wg, wg);
			}
			ptr += gi.second.advance_w;
			stbtt_FreeBitmap(bitmap, nullptr);
		}
	}

	return buffer;
}

tGlyphsInfo FormGlyphsText(
	string_view text,
	const std::vector<Font> fonts)
{
	tGlyphsInfo glyphs;
	std::u32string wtext = vs::UTF8ToUTF32Convert(text);

	for (char32_t code : wtext)
	{
		GlyphInfo gi;

		for (int i = 0; i < fonts.size(); i++)
		{
			if (stbtt_FindGlyphIndex(&fonts[i].fontInfo, code))
			{
				gi.fontIdx = i;
				break;
			}
		}

		const stbtt_fontinfo* fontInfo = &fonts[gi.fontIdx].fontInfo;
		float scale = fonts[gi.fontIdx].scale;

		int lsb(0), advanceWidth(0);
		stbtt_GetCodepointHMetrics(fontInfo, int(code), &advanceWidth, &lsb);

		gi.advance_w = static_cast<int>(std::round(scale * advanceWidth));
		gi.lsb = static_cast<int>(std::round(scale * lsb));

		stbtt_GetCodepointBitmapBox(fontInfo, int(code), scale, scale, &gi.xmin, &gi.ymin, &gi.xmax, &gi.ymax);

		glyphs.emplace_back(int(code), gi);
	}

	return glyphs;
}

void StbttDrawText(string_view text, uint32_t textHeight, uint8_t* image, uint32_t imgWidth, uint32_t imgHeight)
{
	if (imgWidth * imgHeight == 0)
		return;

	std::vector<Font> fonts(2);

	if (!InitFont(default_ttf, textHeight, fonts[0]))
		return;

	if (!InitFont(seguiemj_ttf, textHeight, fonts[1]))
		return;

	tGlyphsInfo glyphs = FormGlyphsText(text, fonts);
	std::vector<tGlyphsInfo> glyphStrings(1);
	std::vector<VSSize> stringsSize(1);
	uint32_t currStrWidth = 0;
	uint32_t maxStringWidth = 0;
	uint32_t maxGlyphHeight = 0;
	uint32_t totalTextHeight = 0;

	for (auto& g : glyphs)
	{
		if (currStrWidth + g.second.advance_w > imgWidth)
		{
			maxStringWidth = std::max(maxStringWidth, currStrWidth);
			totalTextHeight += maxGlyphHeight;

			glyphStrings.emplace_back();
			stringsSize.emplace_back();

			currStrWidth = 0;
			maxGlyphHeight = 0;
		}

		glyphStrings.back().push_back(g);

		maxGlyphHeight = std::max(maxGlyphHeight, uint32_t(g.second.ymax - g.second.ymin));
		currStrWidth += g.second.advance_w;

		stringsSize.back().width = currStrWidth;
		stringsSize.back().height = maxGlyphHeight;
	}

	maxStringWidth = std::max(maxStringWidth, currStrWidth);
	totalTextHeight += maxGlyphHeight;

	VSPoint textCorner{
		static_cast<int32_t>((imgWidth - maxStringWidth) / 2),
		static_cast<int32_t>((imgHeight - totalTextHeight) / 2)
	};
	VSPoint currString;
	VSPlaneView dstImage(image, { imgWidth, imgHeight });

	for (size_t i = 0; i < glyphStrings.size(); i++)
	{
		currString.x = (imgWidth - textCorner.x * 2 - stringsSize[i].width) / 2;

		if (currString.y + stringsSize[i].height > imgHeight)
			break;

		uint32_t glyphX = 0;

		for (auto& gi : glyphStrings[i])
		{
			int glyphW(0), glyphH(0);
			Font& font = fonts[gi.second.fontIdx];
			uint8_t* bitmap = stbtt_GetCodepointBitmap(&font.fontInfo, font.scale, font.scale, gi.first, &glyphW, &glyphH, 0, 0);

			VSPlaneView glyphView(bitmap, { (uint32_t)glyphW, (uint32_t)glyphH });

			VSPoint glyphDst = textCorner + currString;
			glyphDst.y += font.baseline + gi.second.ymin;
			glyphDst.x += glyphX;

			glyphView.CopyTo(dstImage.GetSubview({ glyphDst, glyphView.Size() }));

			stbtt_FreeBitmap(bitmap, nullptr);

			glyphX += gi.second.advance_w;
		}

		currString.y += stringsSize[i].height;
	}
}
