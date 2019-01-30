#include <sqlite3.h>
#include <SensorsDatabase.h>
#include <string>
#include <iostream>

using namespace std;

Statement::Statement(sqlite3* db, string sql) : db(db) {
	const char* ctail;
	int rc = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, &ctail);
	if(rc == SQLITE_OK){
		tail = ctail;
	} else {
		throw_error(rc);
	}
}

Statement::~Statement(){
	sqlite3_finalize(stmt);
}

[[noreturn]]
void Statement::throw_error(int rc){
	sqlite3_finalize(stmt);
	throw DatabaseError(rc);
}

void Statement::reset(){
	int rc = sqlite3_reset(stmt);
	if(rc != SQLITE_OK){
		throw_error(rc);
	}
}

bool Statement::next_statement(void){
	bool next = false;
	const char* ctail = tail.c_str();
	int rc;
	if(!tail.empty()){
		sqlite3_finalize(stmt);

		rc = sqlite3_prepare_v2(db, ctail, tail.length(), &stmt, &ctail);
		if(rc == SQLITE_OK){
			next = true;
			tail = ctail;
		} else {
			throw_error(rc);
		}
	}

	return next;
}

void Statement::bind_value(int id, sqlite3_int64 value){
	int rc = sqlite3_bind_int64(stmt, id, value);
	if(rc != SQLITE_OK){
		throw_error(rc);
	}
}

void Statement::bind_value(int id, double value){
	int rc = sqlite3_bind_double(stmt, id, value);
	if(rc != SQLITE_OK){
		throw_error(rc);
	}
}

void Statement::bind_value(int id, string value){
	int rc = sqlite3_bind_text(
		stmt, id, value.c_str(), value.length(), SQLITE_STATIC);
	if(rc != SQLITE_OK){
		throw_error(rc);
	}
}

void Statement::execute(){
	next_line();
}

bool Statement::next_line(){
	int rc;
	do {
		rc = sqlite3_step(stmt);
	} while(rc == SQLITE_BUSY);

	if(rc == SQLITE_ROW){
		return true;
	} else if(rc == SQLITE_DONE){
		return false;
	} else {
		throw_error(rc);
	}
}

void Statement::get_column_value(int colid, sqlite3_int64 &value){
	value = sqlite3_column_int64(stmt, colid);
}

void Statement::get_column_value(int colid, double& value){
	value = sqlite3_column_double(stmt, colid);
}

void Statement::get_column_value(int colid, string &value){
	auto v = (const char*) sqlite3_column_text(stmt, colid);
	if(v != NULL){
		value = v;
	}
}
