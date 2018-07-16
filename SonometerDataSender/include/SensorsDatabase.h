#ifndef __QARPEDIEM_SENSORS_DATABASE__
#define __QARPEDIEM_SENSORS_DATABASE__

#include <execinfo.h>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <sqlite3.h>
#include <exception>
#include <string>
#include <unordered_map>
#include <ctime>
#include <vector>

class DatabaseError : public std::exception {
private:
	std::string error_str;
	std::string backtrace_str;

	void get_backtrace(){
		void* frames[12];
		int frame_count = backtrace(&frames[0], 12);
		char** symbols_names = backtrace_symbols(frames, frame_count);

		backtrace_str = "Backtrace (last 10 calls):\n";
		for(size_t i = 2; i < frame_count; i++){
			backtrace_str += symbols_names[i];
			backtrace_str += "\n";
		}

		std::cerr << backtrace_str << std::endl;
	}

public:
	DatabaseError(char* sqlite_error){
		error_str = sqlite_error;
		sqlite3_free(sqlite_error);
		get_backtrace();
	}

	DatabaseError(int sqlite_code){
		error_str = sqlite3_errstr(sqlite_code);
		get_backtrace();
	}

	~DatabaseError() throw() {}

	virtual const char* get_backtrace() const throw() {
		return backtrace_str.c_str();
	}

	virtual const char* what() const throw() {
		return error_str.c_str();
	}
};

class Statement {
private:
	sqlite3* db;
	sqlite3_stmt* stmt;
	std::string tail;

	[[noreturn]]
	void throw_error(int rc);

public:
	Statement(sqlite3* db, std::string sql);
	~Statement();

	void reset();
	bool next_statement(void);

	void bind_value(int id, sqlite3_int64 value);
	void bind_value(int id, double value);
	void bind_value(int id, std::string value);

	void execute(void);
	bool next_line();

	void get_column_value(int colid, sqlite3_int64& value);
	void get_column_value(int colid, double& value);
	void get_column_value(int colid, std::string& value);
};

class SensorsDatabase {
private:
	sqlite3* db;
	void init_database(void);
public:
	typedef void (*sensors_cb_t)(
		void* user,
		std::string sensor,
		double sensibility,
		double offset);

	typedef std::unordered_map<std::string, double> poll_t;

	SensorsDatabase(std::string path, bool configure = true);
	~SensorsDatabase(void);

	void create_poll(time_t poll);
	void commit_poll(time_t poll);
	void poll_sent(time_t poll);
	void sonometer_poll_sent(time_t poll);

	void push_data(std::string sensor, time_t poll, double data);

	void delete_old_polls(time_t older_than);
	void delete_sonometer_old_polls(time_t older_than);

	void for_each_sensor(void* user, sensors_cb_t callback);

	std::vector<time_t> get_sendable_polls(size_t max = 100);
	std::vector<time_t> get_sendable_sonometer_polls(size_t max = 100);
	
	poll_t get_poll(time_t poll_time);
	poll_t get_sonometer_poll(time_t poll_time);

	std::string get_config(std::string config);
	void set_config(std::string config, float new_value);
};

#endif // __QARPEDIEM_SENSORS_DATABASE__
