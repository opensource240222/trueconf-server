#pragma once
class VS_RouterMessage;

class VS_RouterMessExtHandlerInterface
{
public:
	virtual bool IsMyMess(VS_RouterMessage* mess) = 0;
	virtual bool ProcessingMessgae(VS_RouterMessage* mess) = 0;
};