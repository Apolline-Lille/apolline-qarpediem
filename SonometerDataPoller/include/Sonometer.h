/**
* Sonometer.h
* Definition of the Sonometer class
*
* Contributor: Sébastien Maes
*/
#ifndef __SONOMETER__
#define __SONOMETER__

#include <iostream>
#include <cstdio> // standard input / output functions
#include <cstdlib>
#include <cstring> // string function definitions
#include <string>
#include <ctime>
#include <cmath>
#include <system_error>
#include <unistd.h>  // UNIX standard function definitions
#include <fcntl.h>   // File control definitions
#include <termios.h> // POSIX terminal control definitions
#include <sys/ioctl.h>
#include <errno.h> // Error number definitions

#include <wiringPi.h>

#include <SensorsDatabase.h>

#define FRAME_SEPARATOR '>'
#define BUFFER_SIZE 128
#define GPIO_PIN 21

using namespace std;

class Sonometer
{

  private:
	int port_fd;
	int nb_bytes;

	// Sonometer Data (mainly, for display purposes)
	double m_la10;
	double m_la50;
	double m_la90;
	double m_avgLaEq;
	double m_freqMax;
	double m_avgCGS;
	double m_nEvtLaEq;
	double m_tEvtLaEq;
	double m_nEvt60db;
	double m_tEvt60db;

	/**
	 * Configure port (baudrate, parity etc.)
	 *
	 * @param fd file descriptor to a serial port
	 *
	 * @return true if the file descriptor was properly configured
	 **/
	bool configurePort(int fd);

	bool isFrameComplete(char *buffer) const;

	bool isBadData(char *buffer);


  public:
	// Constructor
	Sonometer();

	/**
	 * Open a port (e.g /dev/ttyS0 on Linux, COM1 on Windows etc.)
	 *
	 * @param port port designation (/dev/ttyS0, /dev/ttyUSB0...)
	 *
	 * @return true if the port was succesfully opened
	 **/
	bool openPort(string port);

	/**
	 * Close the current port. If it's not opened, we return -1.
	 * If an error occur while closing the port, we return -1 as well. The Linux
	 * kernel assure us that close will always free the descriptor. errno will
	 * be set on case of an error.
	 *
	 * @return true if we succesfully closed the current file descriptor with no
	 * error
	 **/
	bool closePort(void);

	/**
	 * Check if we have a port opened
	 * @return  true if a port is open by Sensors
	 */
	bool isPortOpen(void) const;

	time_t getMonotonicTime(void) const;

	void clear(char *buffer) const;

	void displayData(char *buffer) const;

	bool isInitFrameReceived(char *buffer) const;

	int sendRequest(void);

	int readData(char *buffer);

	void processData(char *buffer, SensorsDatabase *db);
	
};

#endif // __SONOMETER__
