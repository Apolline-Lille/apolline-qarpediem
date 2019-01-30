#ifndef __SOUND_SENSOR__
#define __SOUND_SENSOR__

#include <Arduino.h>

class SensorAverage {
	uint16_t* readings = NULL;
	size_t readings_count = 0;
	size_t reading_position = 0;

	float sampling_frequency = 0;
	float reading_lifetime_seconds = 0;

	void cleanup_readings(void);

public:
	/**
	 * SensorAverage constructor
	 * @method SensorAverage
	 * @param  sampling_freq in Hz
	 * @param  readings_lifetime in seconds
	 */
	SensorAverage(float sampling_freq = 4, float readings_lifetime = 30);

	/**
	 * Set readings sampling frequecy and lifetime
	 * @method set_readings_settings
	 * @param  sampling_freq in Hz
	 * @param  lifetime in seconds
	 */
	bool set_readings_settings(float sampling_freq, float lifetime);

	/**
	 * Return the current sampling period
	 * @method get_sampling_period
	 * @return the current sampling period in seconds or NAN for an infinite
	 * period
	 */
	float get_sampling_period(void);

	/**
	 * Return the average voltage returned by the sound sensor
	 * @method get_average_level
	 * @return average voltage
	 */
	float get_average_level(void);

	/**
	 * Add a value and
	 * @method push_value
	 * @param  value [description]
	 */
	void push_value(uint16_t value);
};

#endif // __SOUND_SENSOR__
