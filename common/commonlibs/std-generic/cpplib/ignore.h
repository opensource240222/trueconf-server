#pragma once

namespace vs {

template <class T>
class ignore
{
public:
	operator       T&()       { return x_; }
	operator const T&() const { return x_; }
private:
	T x_ = {};
};

}
