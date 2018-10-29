#include <Arduino.h>
#include "SensorAverage.h"

SensorAverage::SensorAverage(float sampling_freq, float reading_lifetime){
	if(set_readings_settings(sampling_freq, reading_lifetime)){
		for (size_t i = 0; i < readings_count; i++) {
			readings[i] = 0;
		}
	}
}

void SensorAverage::cleanup_readings(void){
	free(readings);
	readings = NULL;
}

bool SensorAverage::set_readings_settings(float sampling_freq, float lifetime){
	size_t new_readings_count = sampling_freq * lifetime;
	size_t new_size = new_readings_count * sizeof(uint16_t);
	uint16_t* readings_new;

	if(new_readings_count > 0){
		readings_new = (uint16_t*) realloc(readings, new_size);
		if(readings_new == NULL){
			return false;
		}
	} else {
		cleanup_readings();
	}

	readings = readings_new;
	reading_position = 0;
	readings_count = new_readings_count;
	sampling_frequency = sampling_freq;
	reading_lifetime_seconds = lifetime;

	return true;
}

float SensorAverage::get_sampling_period(void){
	if(sampling_frequency != 0){
		return 1 / sampling_frequency;
	} else {
		return NAN;
	}
}

void SensorAverage::push_value(uint16_t value){
	if(readings != NULL){
		if(reading_position >= readings_count){
			reading_position = 0;
		}
		readings[reading_position] = value;
		reading_position++;
	}
}

float SensorAverage::get_average_level(void){
	float average = 0;
	if(readings != NULL){
		for (size_t i = 0; i < readings_count; i++) {
			average += readings[i];
		}

		average /= readings_count;
	}

	return average;
}
