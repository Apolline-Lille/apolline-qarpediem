/**
* main.cpp
* Main function of the Sonometer Data Poller program
*
* Contributor: Sébastien Maes
*/
#include <fstream>
#include <ctime>
#include <csignal>

#include <Sonometer.h>
#include <SensorsDatabase.h>
#include <Timer.h>

using namespace std;

// GLOBAL VARIABLES
SensorsDatabase *database;
Sonometer sonometer;
Timer globalTimer;
static bool polling_enabled = true;
static bool main_loop = true;
static bool retrieve_data = true;

void stop_datapoller(int sig)
{
	retrieve_data = false;
	polling_enabled = false;
	main_loop = false;
}

void enable_polling(int sig)
{
	retrieve_data = true;
	polling_enabled = true;
}

void disable_polling(int sig)
{
	retrieve_data = false;
	polling_enabled = false;
}

void write_pid()
{
	ofstream pid_file("/tmp/sonometer-datapoller.pid", ios::out);
	string pid = to_string(getpid());
	pid_file.write(pid.c_str(), pid.length());
	pid_file.close();
}

int main(int argc, char *argv[])
{
	// VARIABLES DECLARATION
	int poll_count(0);			  // Total number of polls since the execution of the program
	int nb_loop(0);				  // Number of loops until we receive an init frame
	int init(1);				  // To determine if the program just started, in order to execute some specific init procedures
	int ready(0);				  // Used to determine if the sonometer is ready so we can request and read data on the serial port
	char dataBuffer[BUFFER_SIZE]; // Buffer which can contain a data frame or an init frame

	// BUFFER INIT
	sonometer.clear(dataBuffer); // Set buffer bytes to '\0'

	// WIRING PI INIT
	wiringPiSetupGpio();
	pinMode(GPIO_PIN, INPUT);

	// DATABASE PATH
	string database_path("");
	if (argc > 1)
	{
		database_path = argv[1];
	}

	// OPEN DATABASE
	cout << "Opening database " << database_path << endl;
	database = new SensorsDatabase(database_path);

	// PROCESS ID
	write_pid();

	// SIGNALS HANDLING
	signal(SIGUSR1, enable_polling);
	signal(SIGHUP, disable_polling);
	signal(SIGINT, stop_datapoller);

	// PORT + DATA LIFETIME
	time_t data_lifetime = (time_t)stoll(database->get_config("lifetime_data"));
	auto serial_port = database->get_config("sonometer_port");

	// MAIN LOOP
	while (main_loop)
	{
		if (polling_enabled)
		{
			// Removing expired polls
			database->delete_old_polls(time(NULL) - data_lifetime);

			// If the port is not open, we open it
			if (!sonometer.isPortOpen())
			{
				sonometer.openPort(serial_port);
			}

			// Data polling/retrieving start here
			if (init == 1)
			{
				while (ready == 0)
				{
					ready = digitalRead(GPIO_PIN);
				}
				// Read "H#" to make sure the baudrate is OK.
				sonometer.readData(dataBuffer);

				sleep(40);

				sonometer.clear(dataBuffer);

				// Read data
				sonometer.sendRequest();
				sonometer.readData(dataBuffer);

				init = 0;
			}
			else if (init == 0 && poll_count == 0)
			{
				ready = digitalRead(GPIO_PIN);

				if (ready)
				{
					cout << "Searching Init Frame..." << endl;
					sonometer.clear(dataBuffer);
					// Try to resync
					sonometer.sendRequest();
					sonometer.readData(dataBuffer);
				}
			}

			if (sonometer.isInitFrameReceived(dataBuffer) == true)
			{
				// Clear buffer
				sonometer.clear(dataBuffer);
				cout << "Init frame received" << endl;

				if (poll_count == 0)
				sleep(50);
			}

			

			while (retrieve_data)
			{
				ready = digitalRead(GPIO_PIN);

				// Removing expired polls
				database->delete_old_polls(time(NULL) - data_lifetime);

				// Read sound level data
				if (ready)
				{
					sonometer.clear(dataBuffer);

					sonometer.sendRequest();
					sonometer.readData(dataBuffer);
					sonometer.processData(dataBuffer, database);

					sonometer.displayData(dataBuffer);

					poll_count++; // Total number of polls since the beginning of the execution of the program
					nb_loop++;	// Number of loops until we receive an init frame

					cout << "Nb loop: " << nb_loop << endl;
					cout << "Nb polls: " << poll_count << endl;
					cout << "Time elapsed: " << globalTimer.elapsed() << "s" << endl;

				}

				if (sonometer.isInitFrameReceived(dataBuffer) == true)
				{
					break; // Exiting the loop
				}

				// SIGNALS HANDLING
				// We can use an interrupt signal to stop the program at any time
				// (For example, by pressing Ctrl + C)
				signal(SIGINT, stop_datapoller);
			}

		}
		else
		{
			sonometer.closePort();
			pause();
		}
	} // __MAIN LOOP__

	cout << "Closing database..." << endl;
	delete database;

	cout << "Program execution stopped." << endl;

	return EXIT_SUCCESS;
}