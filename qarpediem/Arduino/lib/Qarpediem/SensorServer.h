#ifndef __DUPLEX_SENSORS__
#define __DUPLEX_SENSORS__

#include <DuplexBase.h>
#include <AlphaSense.h>

using namespace InterQarpe;

namespace Qarpediem {

class SensorServer : public DuplexBase {
#ifndef __DOXYGEN__

	HardwareSerial* serial;
	uint32_t last_poll_average;
	AlphaSense::HistogramData dust_histogram;
	uint32_t last_histogram_dust;
	uint32_t last_ping_dust;
  //Address of the sensor requested
  int address = 200;// must be between 1 to 35; 

#endif // __DOXYGEN__

	int write_bytes(uint8_t* buffer, size_t buffer_length);

	int read_bytes(uint8_t* buffer, size_t buffer_length);

	int getAddress(void) ;

	size_t bytes_available();

	uint32_t now_ms(void);

	void configure_adc();

	void on_query(const char* query_str);
	void on_config(String config);
	void on_sensor(String sensor_name);
	void on_sensor_adc(char id);
	void on_sensor_dust(String sensor_name);

	void send_dust_bin_particleCC(String sensor_name);

public:
	SensorServer(HardwareSerial* serial);
	void testeOPCN();
	void init(void);

	void routine(void);
};

};

#endif // __DUPLEX_SENSORS__
