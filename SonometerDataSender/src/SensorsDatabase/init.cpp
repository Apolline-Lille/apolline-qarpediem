#include <SensorsDatabase.h>
#include <string>

static const char* sql_init =
"BEGIN IMMEDIATE;"

//-------------------------------------------------
//	POLLS (CREATE TABLE)
//-------------------------------------------------

"CREATE TABLE IF NOT EXISTS sonometer.sonometer_polls ("
	"created INTEGER PRIMARY KEY,"
	"done INTEGER DEFAULT 0,"
	"sent INTEGER DEFAULT 0"
");"

//-------------------------------------------------
//	CONFIG
//-------------------------------------------------

"CREATE TABLE IF NOT EXISTS config.sensors ("
	"identifier TEXT PRIMARY KEY,"
	"common_name TEXT,"
	"sensibility REAL DEFAULT 1,"
	"offset REAL DEFAULT 0,"
	"unit TEXT DEFAULT 'N.A',"
	"chart_color TEXT DEFAULT '#000000'"
");"

"INSERT OR IGNORE INTO config.sensors (identifier, common_name) VALUES "
	"('avg_cgs', 'Average CGS'),"
	"('n_evt_laeq', 'New Event LAeq'),"
	"('t_evt_laeq', 'T Event LAeq'),"
	"('n_evt_60db', 'New Event 60dB'),"
	"('t_evt_60db', 'T Event 60dB')"
";"

"INSERT OR IGNORE INTO config.sensors"
"(identifier, common_name, unit) VALUES "
	"('la10', 'Equivalent Sound Level (10%)', 'dB'),"
	"('la50', 'Equivalent Sound Level (50%)', 'dB'),"
	"('la90', 'Equivalent Sound Level (90%)', 'dB'),"
	"('avg_laeq', 'Average Equivalent Sound Level', 'dB'),"
	"('fmax', 'Maximum Sound Frequency', 'Hz')"
	
";"

//-------------------------------------------------
//	DATA
//-------------------------------------------------

"CREATE TABLE IF NOT EXISTS sonometer.sonometer_data ("
	"sensor_identifier TEXT NOT NULL,"
	"data NOT NULL,"
	"poll_time INTEGER NOT NULL,"

	"FOREIGN KEY(poll_time) "
	"REFERENCES sonometer_polls(created) "
	"ON DELETE CASCADE"
");"

//-------------------------------------------------
//	QARPEDIEM SETTINGS
//-------------------------------------------------
"CREATE TABLE IF NOT EXISTS config.settings ("
	"name TEXT PRIMARY KEY,"
	"value"
");"

// values are in seconds
"INSERT OR IGNORE INTO config.settings (name, value) VALUES "
	"('sonometer_port', '/dev/ttyUSB0')"
";"

"DELETE FROM sonometer.sonometer_polls WHERE done=0;"

"COMMIT;";

void SensorsDatabase::init_database(void){
	Statement stmt(db, sql_init);
	do {
		stmt.execute();
	} while(stmt.next_statement());
}