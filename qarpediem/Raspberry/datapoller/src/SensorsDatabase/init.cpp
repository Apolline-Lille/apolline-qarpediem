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
	"('dust_pm10', 'PM10 Particles', 'ppm'),"
	"('dust_bin00_partcc',  'bin00 Particle per cubic centimeter', 'cc'),"
	"('dust_bin01_partcc',  'bin01 Particle per cubic centimeter', 'cc'),"
	"('dust_bin02_partcc',  'bin02 Particle per cubic centimeter', 'cc'),"
	"('dust_bin03_partcc',  'bin03 Particle per cubic centimeter', 'cc'),"
	"('dust_bin04_partcc',  'bin04 Particle per cubic centimeter', 'cc'),"
	"('dust_bin05_partcc',  'bin05 Particle per cubic centimeter', 'cc'),"
	"('dust_bin06_partcc',  'bin06 Particle per cubic centimeter', 'cc'),"
	"('dust_bin07_partcc',  'bin07 Particle per cubic centimeter', 'cc'),"
	"('dust_bin08_partcc',  'bin08 Particle per cubic centimeter', 'cc'),"
	"('dust_bin09_partcc',  'bin09 Particle per cubic centimeter', 'cc'),"
	"('dust_bin10_partcc',  'bin10 Particle per cubic centimeter', 'cc'),"
	"('dust_bin11_partcc',  'bin11 Particle per cubic centimeter', 'cc'),"
	"('dust_bin12_partcc',  'bin12 Particle per cubic centimeter', 'cc'),"
	"('dust_bin13_partcc',  'bin13 Particle per cubic centimeter', 'cc'),"
	"('dust_bin14_partcc',  'bin14 Particle per cubic centimeter', 'cc'),"
	"('dust_bin15_partcc',  'bin15 Particle per cubic centimeter', 'cc')"
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
//"('server_host', '193.49.213.132'),"
"INSERT OR IGNORE INTO config.settings (name, value) VALUES "
	"('serial_port', '/dev/ttyS0'),"
	"('node_address', 'NA'),"
	"('lifetime_data', 3600 * 24),"
	"('interval_polling', 30),"
	"('interval_ip', 30),"
	"('interval_lora', 42),"
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
