#pragma once

template <
	typename ValueT,
	typename ResultT,
	template <ValueT> class CaseT,
	ValueT DEFAULT,
	ValueT ... CASES
>
struct ValueListSwitch {
	using value_type = ValueT;
	using result_type = ResultT;
	template <value_type x> using case_type = CaseT<x>;

	template < value_type FIRST, value_type ... REST>
	struct Match
	{
		inline result_type
		choose_by(value_type x)
		{
			switch (x)
			{
			case FIRST:
				return result_type(CaseT<FIRST>{});
			default:
				return Match<REST...>().choose_by(x);
			}
		}
	};

	template <value_type LAST>
	struct Match<LAST>
	{
		inline result_type
		choose_by(value_type x)
		{
			switch (x)
			{
			case LAST:
				return result_type(CaseT<LAST>{});
			default:
				return result_type(CaseT<DEFAULT>{});
			}
		}
	};

	static inline result_type
	choose_by(value_type x) { return Match<CASES...>().choose_by(x); }
};

template <
	typename ValueT,
	typename ResultT,
	template <ValueT> class CaseT,
	ValueT DEFAULT
>
struct ValueListSwitch< ValueT, ResultT, CaseT, DEFAULT >
{ static_assert(true, "ValueSwitch with default case only."); };

template <
	typename ValueT,
	typename ResultT,
	template <ValueT> class CaseT,
	ValueT DEFAULT,
	ValueT LOWER_BOUND,
	ValueT UPPER_BOUND
	// [Lower, Upper) range
>
struct ValueRangeSwitch {

	static_assert(LOWER_BOUND < UPPER_BOUND,
			"ValueRangeSwitch range must be [lower; upper).");

	using value_type = ValueT;
	using result_type = ResultT;
	template <value_type x> using case_type = CaseT<x>;

	template < typename T, ValueT V >
	// T prevents explicit specialization restriction
	struct Match
	{
		inline result_type
		choose_by(value_type x)
		{
			switch (x)
			{
			case V:
				return result_type(CaseT< V >{});
			default:
				return result_type(
					Match< T, previous(V) >{}.choose_by(x)
				);
			}
		}
	};

	template <typename T>
	// T prevents explicit specialization restriction
	struct Match<T, LOWER_BOUND>
	{
		inline result_type
		choose_by(value_type x)
		{
			switch (x)
			{
			case LOWER_BOUND:
				return result_type(CaseT<LOWER_BOUND>{});
			default:
				return result_type(CaseT<DEFAULT>{});
			}
		}
	};

	static inline result_type
	choose_by(value_type x) {
		return Match< void, previous(UPPER_BOUND) >{}.choose_by(x);
	}
};
