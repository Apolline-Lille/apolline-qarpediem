#include <SensorsDatabase.h>
#include <string>
#include <unordered_map>

using namespace std;

// Class constructor
SensorsDatabase::SensorsDatabase(string path, bool configure){
		
	int rc = sqlite3_open("", &db);
	if(rc == SQLITE_OK){
		
		Statement(db, "PRAGMA foreign_keys=on;").execute();
		Statement(db, "PRAGMA journal_mode=WAL;").execute();

		string sensors = "ATTACH DATABASE ";
		sensors += ("'" + path + "/sonometer.db' AS sonometer;");
		cout << sensors << endl;

		string config = "ATTACH DATABASE ";
		config += ("'" + path + "/config.db' AS config;");
		cout << config << endl;

		Statement(db, sensors).execute();
		Statement(db, config).execute();
	

		if(configure){
			init_database();
		}
	} else {
		throw DatabaseError(rc);
	}
}

// Class Destructor
SensorsDatabase::~SensorsDatabase(void){
	sqlite3_close(db);
}

/*
*	void SensorsDatabase::create_poll
*/
static const char* create_poll_sql =
"INSERT INTO sonometer.sonometer_polls (created) VALUES(?);";

void SensorsDatabase::create_poll(time_t poll){
	Statement stmt(db, create_poll_sql);
	stmt.bind_value(1, (sqlite3_int64) poll);
	stmt.execute();
}

/*
*	void SensorsDatabase::push_data_sql
*/
static const char* push_data_sql =
"INSERT INTO sonometer.sonometer_data (sensor_identifier, data, poll_time) VALUES(?,?,?)";

void SensorsDatabase::push_data(string sensor, time_t poll, double data){
	Statement stmt(db, push_data_sql);
	stmt.bind_value(1, sensor);
	stmt.bind_value(2, data);
	stmt.bind_value(3, (sqlite3_int64) poll);
	stmt.execute();
}

/*
*	void SensorsDatabase::commit_poll_sql
*/
static const char* commit_poll_sql =
"UPDATE sonometer.sonometer_polls SET done=1 WHERE created=?;";
void SensorsDatabase::commit_poll(time_t poll){
	Statement stmt(db, commit_poll_sql);
	stmt.bind_value(1, (sqlite3_int64) poll);
	stmt.execute();
}

static const char* get_config_sql =
"SELECT value FROM config.settings WHERE name=?;";
string SensorsDatabase::get_config(string config){
	Statement stmt(db, get_config_sql);
	string value;

	stmt.bind_value(1, config);
	if(stmt.next_line()){
		stmt.get_column_value(0, value);
	}
	return value;
}

static const char* set_config_sql =
"UPDATE config.settings SET value=? WHERE name=?;";
void SensorsDatabase::set_config(string config, float new_value){
	Statement stmt(db, set_config_sql);

	stmt.bind_value(1, (double)new_value);
	stmt.bind_value(2, config);
	stmt.execute();
}

static const char* get_sensors_settings =
"SELECT identifier, sensibility, offset FROM config.sensors;";
void SensorsDatabase::for_each_sensor(void* user, sensors_cb_t callback){
	Statement stmt(db, get_sensors_settings);
	string identifier;
	double sensibility, offset;
	while(stmt.next_line()){
		stmt.get_column_value(0, identifier);
		stmt.get_column_value(1, sensibility);
		stmt.get_column_value(2, offset);
		callback(user, identifier, sensibility, offset);
	}
}


static const char* get_sensor_data_for_poll_sql =
"SELECT sensor_identifier, data FROM sonometer.sonometer_data WHERE poll_time = ?";
SensorsDatabase::poll_t SensorsDatabase::get_poll(time_t poll_time){
	Statement stmt(db, get_sensor_data_for_poll_sql);
	poll_t poll;
	string sensor;
	double data;

	stmt.bind_value(1, (sqlite3_int64) poll_time);
	while(stmt.next_line()){
		stmt.get_column_value(0, sensor);
		stmt.get_column_value(1, data);
		poll[sensor] = data;
	}

	return poll;
}

static const char* get_poll_list_sql =
"SELECT created FROM sonometer.sonometer_polls WHERE done=1 AND sent=0 LIMIT ";
vector<time_t> SensorsDatabase::get_sendable_polls(size_t max){
	string sql = get_poll_list_sql;
	sql += to_string(max);
	sql += ';';
	cout << sql << endl;	

	Statement stmt(db, sql.c_str());
	vector<time_t> polls;
	sqlite3_int64 poll_time;

	while(stmt.next_line()){
		stmt.get_column_value(0, poll_time);
		polls.push_back((time_t) poll_time);
	}

	return polls;
}

static const char* poll_sent_sql =
"UPDATE sonometer.sonometer_polls SET sent=1 WHERE created=?;";
void SensorsDatabase::poll_sent(time_t poll){
	Statement stmt(db, poll_sent_sql);
	stmt.bind_value(1, (sqlite3_int64) poll);
	stmt.execute();
}

static const char* delete_old_polls_sql =
"DELETE FROM sonometer.sonometer_polls WHERE done = 0 OR created < ?;";
void SensorsDatabase::delete_old_polls(time_t older_than){
	Statement stmt(db, delete_old_polls_sql);
	stmt.bind_value(1, (sqlite3_int64) older_than);
	stmt.execute();
}


