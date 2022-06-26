#pragma once

#include <gmock/gmock-matchers.h>
#include <gmock/gmock-generated-matchers.h>

MATCHER_P(AnyBitSet, mask, "Tests if at least one in the mask is set") { return (arg & mask) != 0; }
MATCHER_P(AllBitSet, mask, "Tests if all bits in the mask are set") { return (arg & mask) == mask; }

namespace detail {

template <class T>
T* to_raw_ptr(T* p)
{
	return p;
}

template <class SP, class P = decltype(std::declval<SP>().get()), class = typename std::enable_if<std::is_pointer<P>::value>::type>
P to_raw_ptr(const SP& sp)
{
	return sp.get();
}

}

template <typename Seq>
class SeqEqMatcher
{
public:
	SeqEqMatcher(const Seq& seq)
		: m_seq(seq)
	{
	}

	template <class T>
	bool MatchAndExplain(const T& value, ::testing::MatchResultListener* listener) const
	{
		auto seq_it(std::begin(m_seq));
		const auto seq_end(std::end(m_seq));
		auto value_it(std::begin(value));
		const auto value_end(std::end(value));

		size_t pos = 0;
		for (; seq_it != seq_end && value_it != value_end; ++seq_it, (void)++value_it, ++pos)
			if (*seq_it != *value_it)
			{
				if (listener->IsInterested())
					*listener << "first mismatch at position " << pos;
				return false;
			}
		const bool result = seq_it == seq_end && value_it == value_end;
		if (!result && listener->IsInterested())
			*listener << (value_it == value_end ? "shorter" : "longer") << " than expected";
		return result;
	}

	void DescribeTo(::std::ostream* os) const
	{
		using ::testing::internal::PrintTo;
		*os << "is equal to ";
		PrintTo(m_seq, os);
	}

	void DescribeNegationTo(::std::ostream* os) const
	{
		using ::testing::internal::PrintTo;
		*os << "isn't equal to ";
		PrintTo(m_seq, os);
	}

private:
	const Seq m_seq;
};
template <class Seq>
inline ::testing::PolymorphicMatcher<SeqEqMatcher<Seq>> SeqEq(const Seq& x) { return ::testing::MakePolymorphicMatcher(SeqEqMatcher<Seq>(x)); }

template <typename Seq>
class SeqNeMatcher
{
public:
	SeqNeMatcher(const Seq& seq)
		: m_seq(seq)
	{
	}

	template <class T>
	bool MatchAndExplain(const T& value, ::testing::MatchResultListener* /*listener*/) const
	{
		auto seq_it(std::begin(m_seq));
		const auto seq_end(std::end(m_seq));
		auto value_it(std::begin(value));
		const auto value_end(std::end(value));

		for (; seq_it != seq_end && value_it != value_end; ++seq_it, (void)++value_it)
			if (*seq_it != *value_it)
				return true;
		return !(seq_it == seq_end && value_it == value_end);
	}

	void DescribeTo(::std::ostream* os) const
	{
		using ::testing::internal::PrintTo;
		*os << "isn't equal to ";
		PrintTo(m_seq, os);
	}

	void DescribeNegationTo(::std::ostream* os) const
	{
		using ::testing::internal::PrintTo;
		*os << "is equal to ";
		PrintTo(m_seq, os);
	}

private:
	const Seq m_seq;
};
template <class Seq>
inline ::testing::PolymorphicMatcher<SeqNeMatcher<Seq>> SeqNe(const Seq& x) { return ::testing::MakePolymorphicMatcher(SeqNeMatcher<Seq>(x)); }

template <class InnerMatcher, class T, size_t data_idx, size_t size_idx>
class ArgsAsArrayMatcher
{
	using args_type = std::tuple<const T*, size_t>;
	using inner_matcher_type = ::testing::Matcher<const args_type&>;
public:
	explicit ArgsAsArrayMatcher(const InnerMatcher& inner_matcher)
		: m_inner_matcher(::testing::SafeMatcherCast<const args_type&>(inner_matcher))
	{
	}

	template <class ArgsTuple>
	bool MatchAndExplain(const ArgsTuple& value, ::testing::MatchResultListener* listener) const
	{
		static_assert(data_idx < std::tuple_size<ArgsTuple>::value, "not enough arguments to extract data pointer");
		static_assert(size_idx < std::tuple_size<ArgsTuple>::value, "not enough arguments to extract data size");

		args_type selected_args(
			static_cast<const T*>(detail::to_raw_ptr(std::get<data_idx>(value))),
			static_cast<size_t>(std::get<size_idx>(value))
		);

		if (!listener->IsInterested())
			return m_inner_matcher.Matches(selected_args);

		*listener->stream() << "a tuple whose fields (#" << data_idx << ", #" << size_idx << ") are ";
		::testing::internal::PrintTo(selected_args, listener->stream());
		*listener->stream() << ' ';
		return m_inner_matcher.MatchAndExplain(selected_args, listener);
	}

	void DescribeTo(::std::ostream* os) const
	{
		*os << "are a tuple whose fields (#" << data_idx << ", #" << size_idx << ") describe an array that ";
		m_inner_matcher.DescribeTo(os);
	}

	void DescribeNegationTo(::std::ostream* os) const
	{
		*os << "are a tuple whose fields (#" << data_idx << ", #" << size_idx << ") describe an array that ";
		m_inner_matcher.DescribeNegationTo(os);
	}

private:
	const inner_matcher_type m_inner_matcher;
};
template <class T, size_t data_idx, size_t size_idx, class InnerMatcher>
inline ::testing::PolymorphicMatcher<ArgsAsArrayMatcher<InnerMatcher, T, data_idx, size_idx>> ArgsAsArray(const InnerMatcher& matcher)
{
	return ::testing::MakePolymorphicMatcher(ArgsAsArrayMatcher<InnerMatcher, T, data_idx, size_idx>(matcher));
}

template <class InnerMatcher, class T>
class AsArrayMatcher
{
	using args_type = std::tuple<const T*, size_t>;
	using inner_matcher_type = ::testing::Matcher<const args_type&>;
public:
	explicit AsArrayMatcher(size_t size, const InnerMatcher& inner_matcher)
		: m_inner_matcher(::testing::SafeMatcherCast<const args_type&>(inner_matcher))
		, m_size(size)
	{
	}

	template <class Arg>
	bool MatchAndExplain(const Arg& value, ::testing::MatchResultListener* listener) const
	{
		args_type args(
			static_cast<const T*>(static_cast<const void*>(detail::to_raw_ptr(value))),
			m_size
		);

		if (!listener->IsInterested())
			return m_inner_matcher.Matches(args);

		*listener->stream() << "a pointer to array of size " << m_size << " which is ";
		::testing::internal::PrintTo(args, listener->stream());
		*listener->stream() << ' ';
		return m_inner_matcher.MatchAndExplain(args, listener);
	}

	void DescribeTo(::std::ostream* os) const
	{
		*os << "is a pointer to array os size " << m_size << " that ";
		m_inner_matcher.DescribeTo(os);
	}

	void DescribeNegationTo(::std::ostream* os) const
	{
		*os << "is a pointer to array os size " << m_size << " that ";
		m_inner_matcher.DescribeNegationTo(os);
	}

private:
	const inner_matcher_type m_inner_matcher;
	size_t m_size;
};
template <class T, class InnerMatcher>
inline ::testing::PolymorphicMatcher<AsArrayMatcher<InnerMatcher, T>> AsArray(size_t size, const InnerMatcher& matcher)
{
	return ::testing::MakePolymorphicMatcher(AsArrayMatcher<InnerMatcher, T>(size, matcher));
}
