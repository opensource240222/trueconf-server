#pragma once
struct VS_Overlapped;
class VS_IOHandler
{
public:
	virtual ~VS_IOHandler(){}
	virtual void Handle(const unsigned long sz, const struct VS_Overlapped *ov) = 0;
	virtual void HandleError(const unsigned long err, const struct VS_Overlapped *ov) = 0;
};