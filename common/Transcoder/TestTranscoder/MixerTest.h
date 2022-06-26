#pragma once

#include "Transcoder/mixer/VS_MultiMixerVideo.h"
#include "Dump.h"
#include "std-generic/cpplib/deleters.h"
#include "std/cpplib/layout_json.h"

#include <fstream>

class VSI420SharedImage
{
public:
	VSI420SharedImage(VSSize size)
		: m_data(new uint8_t[size.Square() * 3 / 2], array_deleter<uint8_t>{})
		, m_size(size)
	{
	}

	uint8_t* Data()
	{
		return m_data.get();
	}

	uint32_t Size() const
	{
		return m_size.Square() * 3 / 2;
	}

	VSI420ImageView GetView()
	{
		return { m_data.get(), m_size };
	}

private:
	std::shared_ptr<uint8_t> m_data;
	VSSize m_size;
};

std::vector<std::vector<uint8_t>> ReadSv(const std::string& fname)
{
	std::ifstream inFile(fname, std::ios::binary);

	uint32_t size;
	std::vector<std::vector<uint8_t>> result;

	while (inFile.read((char*)&size, 4).gcount() == 4)
	{
		result.emplace_back(size);
		inFile.read((char*)result.back().data(), size);
	}

	return result;
}

std::vector<VSI420SharedImage> ReadRawVideo(std::string fileName, VSSize size)
{
	std::ifstream inFile(fileName, std::ios::binary);

	std::vector<VSI420SharedImage> result;
	while (true)
	{
		VSI420SharedImage img(size);
		if (inFile.read((char*)img.Data(), img.Size()).gcount() != img.Size())
			break;
		result.push_back(std::move(img));
	}
	return result;
}

std::vector<VSI420SharedImage> CrateVariousResolutionVideo(std::vector<VSI420ImageView>& images, VSSize startSize, VSSize endSize)
{
	int delay = 5;

	std::vector<VSSize> sizes(images.size());

	for (int i = 0; i < sizes.size(); i++)
	{
		float factor = float(i + 1) / sizes.size();
		VSSize s = {
			uint32_t(startSize.width + ((int)endSize.width - (int)startSize.width) * factor) & ~3,
			uint32_t(startSize.height + ((int)endSize.height - (int)startSize.height) * factor) &~ 1 };

		sizes[i] = s;
	}

	std::vector<VSI420SharedImage> result;

	for (int i = 0; i < images.size(); i++)
	{
		VSSize dstSize = sizes[i];
		VSI420SharedImage img(dstSize);

		VSVideoProcessingIpp vp;
		vp.ResampleI420(
			images[i].GetPlaneY().Data(), images[i].Size().width, images[i].Size().height,
			img.Data(), dstSize.width, dstSize.height);

		result.push_back(std::move(img));
	}

	return result;
}

std::vector<VSI420ImageView> ImagesToViews(std::vector<VSI420SharedImage>& images)
{
	std::vector<VSI420ImageView> result;

	for (auto& img : images)
		result.push_back(img.GetView());

	return result;
}

std::vector<std::shared_ptr<media_synch::VideoBuffer>> ImagesToVideoBuffers(std::vector<VSI420ImageView>& images)
{
	std::vector<std::shared_ptr<media_synch::VideoBuffer>> result;

	for (auto& img : images)
	{
		std::shared_ptr<media_synch::VideoBuffer> vb;
		vb.reset(new media_synch::VideoBuffer(img.Size().Square() * 3 / 2, img.Size().width, img.Size().height));

		vb->input_width = vb->width;
		vb->input_height = vb->height;

		std::memcpy(vb->buffer, img.GetPlaneY().Data(), vb->size);

		result.push_back(vb);
	}

	return result;
}

class ImageCycleIterator
{
public:
	ImageCycleIterator(std::vector<std::shared_ptr<media_synch::VideoBuffer>>* images, size_t offset = 0)
		: m_images(images)
		, m_index(offset % m_images->size())
	{
	}

	std::shared_ptr<media_synch::VideoBuffer> Get()
	{
		if (m_index >= m_images->size())
			m_index = 0;

		return m_images->at(m_index++);
	}

	std::vector<std::shared_ptr<media_synch::VideoBuffer>>* m_images = nullptr;
	size_t m_index = 0;
};

void TestMixer()
{
	VSSize testVideoSize = { 640, 360 };
	auto testVideoImages = ReadRawVideo("d:/v/foreman_360p.yuv", testVideoSize);
	auto testVideoViews = ImagesToViews(testVideoImages);
	auto variousResImages = CrateVariousResolutionVideo(testVideoViews, { 640, 260 }, { 360, 640 });
	auto variousResViews = ImagesToViews(variousResImages);
	auto variousResVideoBuffers = ImagesToVideoBuffers(variousResViews);

	struct RayInfo
	{
		std::string name;
		ImageCycleIterator images;
	};

	std::vector<RayInfo> rays;

	for (size_t i = 0; i < 1; i++)
		rays.push_back({ std::string("ray ") + std::to_string(rays.size()), ImageCycleIterator{ &variousResVideoBuffers, i * 10 } });

	VSSize mixerSize = { 640, 360 };

	std::vector<uint8_t> mixerOutData(mixerSize.Square() * 3 / 2);
	std::string dump = "mixer_" + std::to_string(mixerSize.width) + "x" + std::to_string(mixerSize.height) + "_.yuv";

	VS_MultiMixerVideo mixer({ mixerSize.width, mixerSize.height, 16, 9, 4, 2 });
	VS_MediaFormat rayMf;
	rayMf.SetVideo(640, 360);

	for (auto& r : rays)
		mixer.AddRay(r.name, r.name, rayMf);

	mixer.GetVideo(mixerOutData.data(), mixerSize.width, mixerSize.height);
	DumpToFile(dump, mixerOutData);

	for (int i = 0; i < 300; i++)
	{
		for (auto& ray : rays)
		{
			mixer.Add(ray.name, ray.images.Get());
		}

		if (i % 5 == 0)
		{
			rays.push_back({ std::string("ray ") + std::to_string(rays.size()), ImageCycleIterator{ &variousResVideoBuffers } });
			mixer.AddRay(rays.back().name, rays.back().name, rayMf);
		}

		mixer.GetVideo(mixerOutData.data(), mixerSize.width, mixerSize.height);

		AddToFile(dump, mixerOutData);
	}
}

const char testLayout[] = R"({
  "width": 1280,
  "height": 720,
  "fixed": false,
  "display_name_position": "bottom",
  "priority_slot_position": "corner",
  "slots": [
    {
      "id": "1",
      "display_name": "1",
      "type": "user",
      "priority": 0,
      "rect": {
        "x": 0,
        "y": 0,
        "width": 640,
        "height" : 360
      }
    },
    {
      "id": "crash_un0",
      "display_name": "crash_un0",
      "type": "user",
      "priority": 0,
      "rect": {
        "x": 640,
        "y": 0,
        "width": 640,
        "height" : 360
      }
    },
    {
      "id": "crash_un1",
      "display_name": "crash_un1",
      "type": "user",
      "priority": 0,
      "rect": {
        "x": 320,
        "y": 360,
        "width": 640,
        "height" : 360
      }
    }
  ]
}
)";

VSLayoutDesc JsonToLayout(const std::string jsonText)
{
	json::Object data;
	json::Reader reader;
	std::stringstream ss(jsonText);
	reader.Read(data, ss);

	int emptySlotCount(0);
	VSLayoutDesc mixerDesc;

	mixerDesc.width = json::Number(data["width"]);
	mixerDesc.height = json::Number(data["height"]);

	json::Array slots = data[layout_json::slots];

	for (auto it = slots.Begin(); it != slots.End(); ++it)
	{
		json::Object slot = *it;
		json::Object rect = slot["rect"];

		VSRayInfo info;
		info.rect.offset.x = json::Number(rect["x"]);
		info.rect.offset.y = json::Number(rect["y"]);
		info.rect.size.width = json::Number(rect["width"]);
		info.rect.size.height = json::Number(rect["height"]);
		info.displayname = json::String(slot["display_name"]);
		info.priority = json::Boolean(slot["priority"]);

		std::string type = json::String(slot["type"]);

		if (type == "empty")
			info.type = VSRayInfo::RT_EMPTY;
		else if (type == "content")
			info.type = VSRayInfo::RT_CONTENT;

		std::string id = json::String(slot["id"]);

		if (id.empty())
		{
			id = "empty" + std::to_string(emptySlotCount++);
			info.displayname = " ";
		}

		mixerDesc.layout[id] = info;
	}

	return mixerDesc;
}

void TestMixerSetLayout()
{
	VS_MixerGrid mixerGrid = { 854, 480, 16, 9, 8, 2 };
	VS_MultiMixerVideo mixer(mixerGrid);
	VS_MediaFormat mf;
	mf.SetAudio(16000);
	mf.SetVideo(mixerGrid.W, mixerGrid.H);
	std::string filename = "res_" + std::to_string(mixerGrid.W) + "x" + std::to_string(mixerGrid.H) + "_.yuv";

	DumpToFile(filename, nullptr, 0);

	mixer.SetDisplayNamePosition(DNP_BOTTOM);

	/*VSSize testVideoSize = { 640, 360 };
	auto testVideoImages = ReadRawVideo("d:/v/foreman_360p.yuv", testVideoSize);
	auto testVideoViews = ImagesToViews(testVideoImages);*/

	std::vector<uint8_t> videoBuff(mixerGrid.W * mixerGrid.H * 3 / 2);

	// set layout
	mixer.SetLayout(JsonToLayout(testLayout));

	mixer.GetVideo(videoBuff.data(), mixerGrid.W, mixerGrid.H);
	AddToFile(filename, videoBuff);

	// fix layout
	mixer.SetLayoutFixed(true);
	mixer.AddRay("user 3", "user 3", mf);

	mixer.GetVideo(videoBuff.data(), mixerGrid.W, mixerGrid.H);
	AddToFile(filename, videoBuff);
}
