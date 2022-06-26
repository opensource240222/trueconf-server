#pragma once

#include <list>
#include <string>
#include <utility>

class ErrorHolder final
{
public:
	typedef std::list<std::pair<std::string, std::string>> repository;

	void AddError(std::string nameError, std::string messageError);
	void AddAll(ErrorHolder &&);
	bool IsEmpty() const;
	std::string ToString() const;
private:
	repository errors_;
};
