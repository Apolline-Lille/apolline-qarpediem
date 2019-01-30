#include <SensorsDatabase.h>
#include <Sensors.h>
#include <fstream>
#include <ctime>
#include <unistd.h>
#include <string>
#include <csignal>

using namespace std;

SensorsDatabase* database;
Sensors sensors;
static bool polling_enabled = false;
static bool main_loop = true;

void stop_datapoller(int sig){
	main_loop = false;
}

void enable_polling(int sig){
	polling_enabled = true;
}

void disable_polling(int sig){
	polling_enabled = false;
}

void write_pid(){
	ofstream pid_file("/tmp/qarpediem-datapoller.pid", ios::out);
	string pid = to_string(getpid());
	pid_file.write(pid.c_str(), pid.length());
	pid_file.close();
}

void poll_sensors();

int main(int argc, char* argv[]){
	string database_path = "";
	if(argc > 1){
		database_path = argv[1];
	}

	cout << "opening database " << database_path << endl;
	database = new SensorsDatabase(database_path);

	write_pid();

	signal(SIGUSR1, enable_polling);
	signal(SIGHUP, disable_polling);
	signal(SIGINT, stop_datapoller);
	
	time_t data_lifetime = (time_t) stoll(database->get_config("lifetime_data"));
	auto serial_port = database->get_config("serial_port");

	while(main_loop){
		if(polling_enabled){
			database->delete_old_polls(time(NULL) - data_lifetime);
			if(!sensors.is_port_open()){
				sensors.open_port(serial_port);
			}

			sensors.routine();
			poll_sensors();
			usleep(5000);
		} else {
			sensors.close_port();
			pause();
		}
	}

	cout << "closing database" << endl;
	delete database;

	return EXIT_SUCCESS;
}
