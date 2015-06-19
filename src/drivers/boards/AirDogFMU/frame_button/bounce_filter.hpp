#pragma once

namespace frame_kbd {

template <typename mask_t = unsigned>
struct BounceFilter
{
	/*
	 * Goals are:
	 *  + to debounce button signals and
	 *  + to do it in the same way for matrix and separate pins.
	 *
	 * Requirements:
	 *  + pressed buttons are marked with ones and released with zores.
	 */
	mask_t filtered_1, raw_1;

	BounceFilter()
		: filtered_1{0}, raw_1{0}
	{}

	mask_t
	operator () (mask_t raw_0)
	{
		mask_t filtered_0;
		// filter_0 = filtered_1 & raw_1
		//          | filtered_1 & raw_0
		//          | raw_1 & raw_0;
		filtered_0 = raw_1 & raw_0 | filtered_1 & (raw_1 | raw_0);
		filtered_1 = filtered_0;
		raw_1 = raw_0;
		return filtered_0;
	}
};

} // end of namespace frame_kbd
