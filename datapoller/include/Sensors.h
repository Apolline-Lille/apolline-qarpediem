#ifndef __DUPLEX_SENSORS__
#define __DUPLEX_SENSORS__

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <ctime>
#include <system_error>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <InterQarpe.h>
#include <errno.h>
#include <SensorsDatabase.h>
#include <Sensors.h>

using namespace InterQarpe;
using namespace std;

class Sensors : public DuplexBase {
	int port_fd;

	/**
	 * Configure port (baudrate, parity etc.)
	 *
	 * @param fd file descriptor to a serial port
	 *
	 * @return true if the file descriptor was properly configured
	 **/
	bool configure_port(int fd);

	int write_bytes(uint8_t* buffer, size_t buffer_length);

	int read_bytes(uint8_t* buffer, size_t buffer_length);

	size_t bytes_available();

	uint32_t now_ms(void);

public:
	Sensors();

	/**
	 * Open a port (e.g /dev/ttyS0 on Linux, COM1 on Windows etc.)
	 *
	 * @param port port designation (/dev/ttyS0, /dev/ttyUSB0...)
	 *
	 * @return true if the port was succesfully opened
	 **/
	bool open_port(string port);

	/**
	 * Close the current port. If it's not opened, we return -1.
	 * If an error occur while closing the port, we return -1 as well. The Linux
	 * kernel assure us that close will always free the descriptor. errno will
	 * be set on case of an error.
	 *
	 * @return true if we succesfully closed the current file descriptor with no
	 * error
	 **/
	bool close_port(void);

	/**
	 * Check if we have a port opened
	 * @return  true if a port is open by Sensors
	 */
	bool is_port_open(void);
};

#endif // __DUPLEX_SENSORS__
