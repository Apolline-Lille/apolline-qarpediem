#include <SensorsDatabase.h>
#include <Sensors.h>
#include <string>
#include <ctime>
#include <csignal>
#include <exception>

using namespace std;

extern SensorsDatabase* database;
extern Sensors sensors;

/**
 * Get the time from the MONOTONIC clock. This clock is not affected by
 * discontinous jump in time. Check manual for clock_gettime to get more info
 * @return  timestamp in seconds from the MONOTONIC clock
 */
static time_t monotonic_time(void){
	struct timespec spec;
	if(clock_gettime(CLOCK_MONOTONIC, &spec) != 0){
		throw system_error(errno, system_category());
	}
	return spec.tv_sec;
}

static
void poll_sensor(void* poll_ptr, string id, double sensibility, double offset){
	time_t* poll = static_cast<time_t*>(poll_ptr);
	string query = "sensors::" + id;
	QueryResult result = sensors.query(query.c_str());
	float data;

	if(result.get_data(&data)){
		cout << id << ": " << data << endl;
		data = (data - offset) / sensibility;
		database->push_data(id, *poll, data);
	}
}

void poll_sensors(){
	static time_t last_poll = monotonic_time();

	time_t poll_interval = stoll(database->get_config("interval_polling"));
	time_t now = monotonic_time();
	time_t poll_time;

	try {
		if(now - last_poll > poll_interval){
			poll_time = time(NULL);
			cout << "polling " << poll_time << endl;
			database->create_poll(poll_time);
			database->for_each_sensor(&poll_time, poll_sensor);
			database->commit_poll(poll_time);
			last_poll = now;
		}
	} catch(std::exception& e){
		cout << "failed to poll sensors" << endl << e.what() << endl;
	}
}

