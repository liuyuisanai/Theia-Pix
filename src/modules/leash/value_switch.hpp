#pragma once

template <
	typename ValueT,
	template <ValueT> class CaseHandlerT,
	ValueT ... cases
>
struct ValueSwitch {
	using value_type = ValueT;

	template <value_type x>
	using case_type = CaseHandlerT<x>;

	template < value_type FIRST, value_type ... REST>
	struct CaseFirst
	{
		inline void apply_if_match (value_type x)
		{
			CaseFirst<FIRST> case_first;
			if (case_first.match(x)) case_first.apply();
			else CaseFirst<REST...>().apply_if_match(x);
		}
	};

	template <value_type V>
	struct CaseFirst<V>
	{
		inline bool match(value_type x) { return x == V; }
		inline void apply() { case_type<V>()(); }
		inline void apply_if_match(value_type x) { if (match(x)) apply(); }
	};

	void process(value_type x)
	{ CaseFirst<cases...>().apply_if_match(x); }
};
