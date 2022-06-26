#include "VS_MultiMixerVideo.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "Transcoder/VSVideoUtils.h"

VS_MultiMixerVideo::VS_MultiMixerVideo(const VS_MixerGrid& CustomMixerGrid)
{
	m_stMixerGrid = CustomMixerGrid;

	m_WindowGrigManager.Init(m_stMixerGrid.W, m_stMixerGrid.H, float(m_stMixerGrid.arW) / float(m_stMixerGrid.arH));
	m_WindowGrigManager.SetWindowOffsetMultiplicity(2, 2);
	m_WindowGrigManager.SetWidowMultiplicity(4, 2);

	m_outFormat.SetVideo(m_stMixerGrid.W, m_stMixerGrid.H, 0);

	m_fillBackGndColor = 0x3c;
	VS_RegistryKey key(false, "Transceiver");
	key.GetValue(&m_fillBackGndColor, 4, VS_REG_INTEGER_VT, "Muxer BackGnd Color");

	m_internalFrame.resize(m_outFormat.dwVideoWidht * m_outFormat.dwVideoHeight * 3 / 2);
}

VS_MultiMixerVideo::~VS_MultiMixerVideo()
{
}

bool VS_MultiMixerVideo::Add(const std::string& handle, const std::shared_ptr<media_synch::VideoBuffer>& video)
{
	auto ii = m_mr.find(handle);
	if (ii == m_mr.end())
		return false;

	ii->second->AddVideo(video);
	m_InternalFrameActual = false;

	return true;
}

uint32_t VS_MultiMixerVideo::GetVideo(uint8_t* buff, uint32_t width, uint32_t height)
{
	if (m_mr.empty())
		return 0;

	if (m_NeedEmptyFrame)
	{
		FillEmptyFrame(m_internalFrame.data(), m_outFormat.dwVideoWidht, m_outFormat.dwVideoHeight, m_fillBackGndColor);
		m_NeedEmptyFrame = false;
	}

	if (!m_InternalFrameActual)
	{
		std::shared_ptr<VS_VideoRay> priorRay = m_PriorityRay.lock();
		bool forceDrawOrdinary = false;

		if (priorRay)
		{
			if (m_WindowGrigManager.GetPriorityLayoutType() == VS_WindowGrid::PRIORITY_LAYOUT_OVERLAY)
				forceDrawOrdinary = priorRay->IsNeedDraw();

			priorRay->TranscDrawVideo(m_internalFrame.data(), false);
		}

		for (auto& ii : m_mr)
		{
			if (ii.second != priorRay)
				ii.second->TranscDrawVideo(m_internalFrame.data(), forceDrawOrdinary);
		}

		m_InternalFrameActual = true;
	}

	VSSize outSize{ width, height };

	if (m_outFormat.dwVideoWidht == outSize.width && m_outFormat.dwVideoHeight == outSize.height)
	{
		memcpy(buff, m_internalFrame.data(), m_internalFrame.size());
	}
	else
	{
		m_videoProcessing.ResampleI420(
			m_internalFrame.data(), m_outFormat.dwVideoWidht, m_outFormat.dwVideoHeight,
			buff, outSize.width, outSize.height);
	}

	for (auto& ii : m_mr)
		ii.second->DrawDisplayName(buff, outSize);

	return m_outFormat.dwVideoWidht * m_outFormat.dwVideoHeight * 3 / 2;
}

int VS_MultiMixerVideo::GetMuxerOutWidth()
{
	return m_stMixerGrid.W;
}

int VS_MultiMixerVideo::GetMuxerOutHeight()
{
	return m_stMixerGrid.H;
}

mixerVideoRaySize VS_MultiMixerVideo::GetMuxerRaysSize()
{
	mixerVideoRaySize mapRay;
	for (auto &ii : m_mr) {
		auto size = ii.second->GetFrameSize();
		if (size.first > 0 && size.second > 0) {
			mapRay[ii.first] = size;
		}
	}
	return mapRay;
}

std::vector<VS_WindowGrid::sOneWindow> VS_MultiMixerVideo::GetRaysGrid()
{
	std::vector<VS_WindowGrid::sOneWindow> grid;
	auto priorityRay = m_PriorityRay.lock();

	for (auto& it : m_mr)
	{
		VS_WindowGrid::sOneWindow win;
		win.userId = it.first;

		VSRect rect = it.second->GetOutputRect();
		win.rect.left = rect.offset.x;
		win.rect.top = rect.offset.y;
		win.rect.right = win.rect.left + rect.size.width;
		win.rect.bottom = win.rect.top + rect.size.height;

		if (it.second == priorityRay)
			win.priority = VS_WindowGrid::PRIORITY_HIGH;
		else
			win.priority = VS_WindowGrid::PRIORITY_LOW;

		grid.push_back(win);
	}

	return grid;
}

void VS_MultiMixerVideo::SetVideoDummy(const std::string& handle, uint8_t * video, size_t width, size_t height)
{
	auto it = m_mr.find(handle);

	if (it != m_mr.end())
		it->second->SetVideoDummy(video, width, height);
}

void VS_MultiMixerVideo::SetRayRect(const std::string & handle, int x, int y, int w, int h)
{
	auto it = m_mr.find(handle);

	if (it != m_mr.end())
		it->second->SetOutputRect({ x, y, (uint32_t)w, (uint32_t)h });

	m_NeedEmptyFrame = true;
}

void VS_MultiMixerVideo::SetLayoutFixed(bool fixed)
{
	m_LayoutFixed = fixed;
	Refresh();
}

void VS_MultiMixerVideo::SetLayout(const VSLayoutDesc & layout)
{
	auto correctedLayout = layout.layout;

	for (auto& r : correctedLayout)
	{
		r.second.rect.offset.x = (r.second.rect.offset.x * m_outFormat.dwVideoWidht / layout.width) &~1;
		r.second.rect.offset.y = (r.second.rect.offset.y * m_outFormat.dwVideoHeight / layout.height) &~1;
		r.second.rect.size.width = (r.second.rect.size.width * m_outFormat.dwVideoWidht / layout.width) &~3;
		r.second.rect.size.height = (r.second.rect.size.height * m_outFormat.dwVideoHeight / layout.height) &~1;
	}

	SetDisplayNamePosition(layout.displayNamePosition);

	ApplyLayout(correctedLayout);
}

void VS_MultiMixerVideo::SetDisplayNamePosition(EDisplayNamePosition position)
{
	m_displayNamePosition = position;

	for (auto& ray : m_mr)
		ray.second->SetDisplayNamePosition(m_displayNamePosition);
}

void VS_MultiMixerVideo::Refresh()
{
	CalculateLayout();
}

bool VS_MultiMixerVideo::AddRay(const std::string& handle, const std::string& name, const VS_MediaFormat& in)
{
	if (m_mr.size() >= LIMIT_CUR_STREAMS)
		return false;

	auto it = m_mr.find(handle);

	if (it != m_mr.end())
	{
		it->second->SetVideoReceive(true);
		if (!m_LayoutFixed) {
			it->second->SetDisplayName(name);
		}
		return true;
	}

	std::shared_ptr<VS_VideoRay> ray(CreateRay(name, m_outFormat, m_fillBackGndColor));

	auto res = m_mr.insert({ handle, ray });

	if (res.second) {
		m_WindowGrigManager.AddWindow(handle);
	}

	CalculateLayout();

	if (res.second)
	{
		res.first->second->SetInputFormat(in);
		res.first->second->SetVideoReceive(true);
		res.first->second->SetDisplayNamePosition(m_displayNamePosition);
	}

	return res.second;
}

bool VS_MultiMixerVideo::RenameRay(const std::string& handle, const std::string& name)
{
	if (m_LayoutFixed) {
		return false;
	}
	auto ii = m_mr.find(handle);
	if (ii == m_mr.end())
		return false;
	ii->second->SetDisplayName(name);
	return true;
}

bool VS_MultiMixerVideo::DeleteRay(const std::string& handle)
{
	auto ii = m_mr.find(handle);
	if (ii == m_mr.end())
		return false;

	if (!m_LayoutFixed)
	{
		m_WindowGrigManager.RemoveWindow(handle);
		m_mr.erase(ii);
		CalculateLayout();
		return true;
	}
	else
	{
		ii->second->SetVideoReceive(false);
		return false;
	}
}

void VS_MultiMixerVideo::CalculateLayout()
{
	if (!m_LayoutFixed)
	{
		std::map<std::string, VSRayInfo> newLayout;

		if (m_mr.size())
		{
			for (const auto& w : m_WindowGrigManager.GetAllWindows())
			{
				VSRect rayState = {
					w.rect.left, w.rect.top,
					uint32_t(w.rect.right - w.rect.left), uint32_t(w.rect.bottom - w.rect.top)
				};

				VSRayInfo ri;
				ri.rect = rayState;
				ri.priority = w.priority;

				newLayout[w.userId] = ri;
			}
		}

		ApplyLayout(newLayout);
	}
}

void VS_MultiMixerVideo::ApplyLayout(const std::map<std::string, VSRayInfo>& layout)
{
	for (auto& ray : m_mr)
	{
		if (!layout.count(ray.first))
			ray.second->SetOutputRect({ 0, 0, 0, 0 });
	}

	for (const auto& i : layout)
	{
		auto ray = m_mr.find(i.first);

		if (ray == m_mr.end())
		{
			std::shared_ptr<VS_VideoRay> newRay(CreateRay(i.second.displayname, m_outFormat, m_fillBackGndColor));

			auto res = m_mr.insert({ i.first, newRay });
			ray = res.first;

			ray->second->SetVideoReceive(false);
		}

		ray->second->SetOutputRect(i.second.rect);
		ray->second->SetDisplayNamePosition(m_displayNamePosition);

		if (i.second.priority)
			m_PriorityRay = ray->second;
	}

	m_NeedEmptyFrame = true;
	m_InternalFrameActual = false;
}

VS_VideoRay* VS_MultiMixerVideo::CreateRay(const std::string& dname, const VS_MediaFormat& out, int backgndColor)
{
	return new VS_VideoRay(dname, out, backgndColor);
}
