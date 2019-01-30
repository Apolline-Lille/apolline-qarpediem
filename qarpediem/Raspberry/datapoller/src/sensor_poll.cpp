#include <SensorsDatabase.h>
#include <Sensors.h>
#include <string>
#include <ctime>
#include <csignal>
#include <exception>
#include <iostream>

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
	return spec.tv_sec + spec.tv_nsec / 1e9;
}

static uint32_t now_millisec() {
	struct timespec spec;
	if(clock_gettime(CLOCK_MONOTONIC, &spec) != 0){
		throw system_error(errno, system_category());
	}
	return spec.tv_sec * 1e3 + spec.tv_nsec / 1e6;
}

static void poll_sensor(void* poll_ptr, string id, double sensibility, double offset){
	time_t* poll = static_cast<time_t*>(poll_ptr);
	string query = "sensors::" + id;
	if(sensors.startsWith(id,"sound") && id.length()>7){
		return; // we exclude sound_xxx
	} 

	// Flush serial port
	usleep(100000);
	tcflush(sensors.getFileDescriptor(), TCIFLUSH);
	
	// Query
	QueryResult result = sensors.query(query.c_str());
	int counter=0;
	while(counter <3 && !sensors.getIs_remote_address_valid() ){
		cout<<"we have not the good response for  " <<query;
		cout<<" ------------> retry : "<<  counter+1<<"/3"<<endl;
		result = sensors.query(query.c_str());
		counter++;
	}// end of while
	
	if(!sensors.getIs_remote_address_valid()) {
		cerr<<"so we leave this poll and do not store any thing"<<endl;
		return;
	} 

	float data;
	static time_t last_query = now_millisec();

	while(!result.get_data(&data))
	{
		time_t now = now_millisec();
		if(now - last_query > 30) break; // 30 ms
	}

	if(result.get_data(&data)){ // 1st try
		cout << id << ": " << data << endl;
		data = (data - offset) / sensibility;
		database->push_data(id, *poll, data);	
	}

	
} // End poll_sensor()

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

