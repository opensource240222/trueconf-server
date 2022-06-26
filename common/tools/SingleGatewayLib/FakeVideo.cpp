#include "FakeVideo.h"
#include "IppLib2/VSVideoProcessingIpp.h"
#include "std-generic/cpplib/VS_Container.h"
#include "Transcoder/StbttDrawText.h"
#include "commonlibs/extlibs/stb/stb_image.h"
#include "std-generic/cpplib/utf8.h"

#include <cstring>
#include <memory>
#include <fstream>

const char* GetFakeVideoMessage(FakeVideo_Mode mode, const char* language)
{
	if (!language||!*language)
		language = "en";

	switch(mode)
	{
	case FVM_GROUPCONF_NOPEOPLE:
		if (std::strcmp(language, "ru") == 0)
			return "\xD0\x92 \xD0\xBA\xD0\xBE\xD0\xBD\xD1\x84\xD0\xB5\xD1\x80\xD0\xB5\xD0\xBD\xD1\x86\xD0\xB8\xD0\xB8 \xD0\xBD\xD0\xB5\xD1\x82 \xD0\xB4\xD1\x80\xD1\x83\xD0\xB3\xD0\xB8\xD1\x85 \xD1\x83\xD1\x87\xD0\xB0\xD1\x81\xD1\x82\xD0\xBD\xD0\xB8\xD0\xBA\xD0\xBE\xD0\xB2"; // "В конференции нет других участников";
		else
			return "There are no participants in Group Conference now";
	case FVM_BROADCAST_INPROGRESS:
		return "BroadCast in progress";
	case FVM_NOSPEAKERS:
		if (std::strcmp(language, "ru") == 0)
			return "\xD0\x9D\xD0\xB0 \xD1\x82\xD1\x80\xD0\xB8\xD0\xB1\xD1\x83\xD0\xBD\xD0\xB5 \xD0\xBD\xD0\xB8\xD0\xBA\xD0\xBE\xD0\xB3\xD0\xBE \xD0\xBD\xD0\xB5\xD1\x82"; // "На трибуне никого нет";
		else
			return "There are no speakers";
	}
	return "";
}

void DrawTextOnImage(VS_BinBuff& imageI420, unsigned int w, unsigned int h, const char* text, unsigned int font_h, unsigned int /*text_w*/, unsigned int /*text_h*/)
{
	imageI420.SetSize(w*h*3/2);
	memset((void*)imageI420.Buffer(), 0, w * h);
	memset((char*)(imageI420.Buffer()) + w * h, 128, w * h / 2);

	StbttDrawText(text, font_h, static_cast<uint8_t*>(imageI420.Buffer()), w, h);
}

bool ReadImageFromFile(VS_BinBuff& imageI420, unsigned int& w, unsigned int& h, const char* path)
{
	::std::vector<uint8_t> preparedData;

	// load RGB image data from downloaded image file
	{
		int lw = 0, lh = 0;
		int loaded_channels = 0;

#if defined(_WIN32)
		auto path_wide = vs::UTF8ToWideCharConvert(path);
		std::ifstream file(path_wide, std::ios::ate | std::ios::binary);
#else
		std::ifstream file(path, std::ios::ate | std::ios::binary);
#endif
		if (!file.is_open()) {
			return false;
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<uint8_t> buffer(fileSize);

		file.seekg(0);
		file.read((char*)buffer.data(), fileSize);

		file.close();

		auto image = stbi_load_from_memory(buffer.data(),
			buffer.size(),
			&lw, &lh, &loaded_channels,
			STBI_rgb); // 24 bits per channel (3 bytes)

		w = static_cast<size_t>(lw);
		h = static_cast<size_t>(lh);

		if (image == nullptr)
			return false;

		// flip image vertically and normalise image size (crop a bit) to make it suitable for conversion to YUV 4:2:0
		{
			const size_t bytes_per_line = w * 3; // 3 bytes per channel (24 bits)
			const size_t normalised_bytes_per_line = (w & ~7) * 3; // 3 bytes per channel (24 bits)
			const size_t image_size = h * bytes_per_line;
			const size_t normalised_image_size = (h & ~7) * normalised_bytes_per_line;
			preparedData.resize(normalised_image_size);
			for (size_t i = 0; i < (h & ~7); i++)
			{
				auto *to = &preparedData[i * normalised_bytes_per_line];
				auto from = &image[image_size - ((i + 1) * bytes_per_line)];
				memcpy(to, from, normalised_bytes_per_line);
			}

			w &= ~7;
			h &= ~7;
		}

		stbi_image_free(image);
	}

	// convert image to YUV 4:2:0
	imageI420.SetSize(w * h / 2 * 3);
	//VS_VideoProc prc;
	VSVideoProcessingIpp prc;
	uint8_t *Y = (uint8_t*)imageI420.Buffer();
	uint8_t *U = Y + w * h;
	uint8_t *V = U + w * h / 4;
	auto res = prc.ConvertRGB24ToI420(&preparedData[0], Y, U, V, w * 3, h, w);

	if (!res)
		imageI420.Empty();

	return res;
}
