
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
	void set_cutoff_frequency(float cutoff_freq) { _rc = (cutoff_freq > 0.0f) ? (1.0f / M_TWOPI_F / cutoff_freq) : 0.0f; }

	/**
	 * Add a new value to the filter
	 *
	 * @return retrieve the filtered result
	 */
	T apply(uint64_t t, const T &value)
	{
		if (_rc > 0.0f) {
			float dt = (t - _time_last) * 1.0e-6f;
			float a = dt / (_rc + dt);
			_value = value * a + _value * (1.0f - a);
		}

		_time_last = t;
		return _value;
	}

	/**
	 * Return the cutoff frequency
	 */
	float get_cutoff_freq(void) const { return (_rc > 0.0f) ? (1.0f / M_TWOPI_F / _rc) : 0.0f; }

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
