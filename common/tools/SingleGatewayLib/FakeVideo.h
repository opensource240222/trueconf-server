#pragma once

class VS_BinBuff;

enum FakeVideo_Mode : int
{
	FVM_DISABLED,
	FVM_GROUPCONF_NOPEOPLE,
	FVM_BROADCAST_INPROGRESS,
	FVM_NOSPEAKERS,
	FVM_SLIDESHOW,
};

const char* GetFakeVideoMessage(FakeVideo_Mode mode, const char* language = nullptr);
void DrawTextOnImage(VS_BinBuff& imageI420, unsigned int w, unsigned int h, const char* text, unsigned int font_h = 12, unsigned int text_w = 0, unsigned int text_h = 0);
bool ReadImageFromFile(VS_BinBuff& imageI420, unsigned int& w, unsigned int& h, const char* path);
