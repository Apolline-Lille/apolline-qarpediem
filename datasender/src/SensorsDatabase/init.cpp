#include <SensorsDatabase.h>
#include <string>

static const char* sql_init =
"BEGIN IMMEDIATE;"

"CREATE TABLE IF NOT EXISTS sensors.polls ("
	"created INTEGER PRIMARY KEY,"
	"done INTEGER DEFAULT 0,"
	"sent INTEGER DEFAULT 0"
");"

"CREATE TABLE IF NOT EXISTS config.sensors ("
	"identifier TEXT PRIMARY KEY,"
	"common_name TEXT,"
	"sensibility REAL DEFAULT 1,"
	"offset REAL DEFAULT 0,"
	"unit TEXT DEFAULT 'N.A',"
	"chart_color TEXT DEFAULT '#000000'"
");"

"INSERT OR IGNORE INTO config.sensors (identifier, common_name) VALUES "
	"('motion', 'Motion'),"
	"('sound', 'Sound Level'),"
	"('luminosity', 'Luminosity')"
";"

"INSERT OR IGNORE INTO config.sensors"
"(identifier, common_name, unit) VALUES "
	"('humidity', 'Humidity', '%'),"
	"('pressure', 'Pressure', 'HectoPascal'),"
	"('temperature', 'Temperature', '°C'),"
	"('aps1', 'AlphaSense 1', 'ppb'),"
	"('aps2', 'AlphaSense 2', 'ppb'),"
	"('aps3', 'AlphaSense 3', 'ppb'),"
	"('aps4', 'AlphaSense 4', 'ppb'),"
	"('aps5', 'AlphaSense 5', 'ppb'),"
	"('aps6', 'AlphaSense 6', 'ppb'),"
	"('aps7', 'AlphaSense 7', 'ppb'),"
	"('aps8', 'AlphaSense 8', 'ppb'),"
	"('co2', 'CO2', 'ppm'),"
	"('dust_pm1', 'PM1 Particles', 'ppm'),"
	"('dust_pm2.5', 'PM2.5 Particles', 'ppm'),"
	"('dust_pm10', 'PM10 Particles', 'ppm')"
";"

"CREATE TABLE IF NOT EXISTS sensors.data ("
	"sensor_identifier TEXT NOT NULL,"
	"data NOT NULL,"
	"poll_time INTEGER NOT NULL,"

	"FOREIGN KEY(poll_time) "
	"REFERENCES polls(created) "
	"ON DELETE CASCADE"
");"

"CREATE TABLE IF NOT EXISTS config.settings ("
	"name TEXT PRIMARY KEY,"
	"value"
");"

// values are in seconds
"INSERT OR IGNORE INTO config.settings (name, value) VALUES "
	"('serial_port', '/dev/ttyS0'),"
	"('node_address', 'NA'),"
	"('lifetime_data', 3600 * 24),"
	"('interval_polling', 30),"
	"('interval_ip', 30),"
	"('interval_lora', 42),"
	"('eui', NA),"
	"('device_eui', NA),"
	"('device_addr', NA),"
	"('send_mode', 'ip'),"
	"('server_host', 'localhost'),"
	"('server_port', 5555)"
";"

"DELETE FROM sensors.polls WHERE done=0;"

"COMMIT;";

void SensorsDatabase::init_database(void){
	Statement stmt(db, sql_init);
	do {
		stmt.execute();
	} while(stmt.next_statement());
}