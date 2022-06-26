#pragma once
namespace chat
{
template<
	typename StrandT,
	typename TimerT = void
>
struct LayersTraits
{
	using StrandType = StrandT;
	using TimerType = TimerT;
};
}