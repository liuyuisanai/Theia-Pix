
#pragma once

namespace math
{

template <typename T>
class __EXPORT LowPassFilter
{
public:
	/**
	 * Constructor with filtering disabled
	 */
	LowPassFilter() : _rc(0.0f), _time_last(0)
	{
	}

	/**
	 * Constructor with cutoff frequency
	 */
	explicit LowPassFilter(float cutoff_freq) : _time_last(0)
	{
		set_cutoff_frequency(cutoff_freq);
	}

	/**
	 * Change filter parameters
	 */
	void set_cutoff_frequency(float cutoff_freq) {
		if (cutoff_freq <= 0.0f)
			_rc = 0.0f;
		else
			_rc = 1.0f / M_TWOPI_F / cutoff_freq;
	}

	/**
	 * Return the cutoff frequency
	 */
	float get_cutoff_freq(void) const {
		if (_rc > 0.0f)
			return 1.0f / M_TWOPI_F / _rc;
		else
			return 0.0f;
	}

	/**
	 * Add a new value to the filter
	 *
	 * @return retrieve the filtered result
	 */
	const T &apply(uint64_t t, const T &next_value) {
		if (_rc > 0.0f) {
			float dt = (t - _time_last) * 1.0e-6f;
			float a = dt / (_rc + dt);
			_value *= (1.0f - a);
			_value += next_value * a;
			if (_time_last == 0) {
				_value = next_value; // init first sample
			}
		}

		_time_last = t;
		return _value;
	}

	/**
	 * Reset the filter state to this value
	 */
	void reset(T value) { _value = value; }

private:
	float _rc;
	uint64_t _time_last;
	T _value;
};

} // namespace math
