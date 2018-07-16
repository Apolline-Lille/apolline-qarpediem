#include <SensorsDatabase.h>
#include <string>
#include <exception>
#include <unistd.h>
#include <csignal>
#include <curl/curl.h>
#include <fstream>

using namespace std;

SensorsDatabase* database;
static bool main_loop = true;

void runtime_lorawan(void);
void runtime_ip(void);
bool sync_time_ip(void);

static pid_t get_poller_pid(void){
	string pid_str;
	const char* pid_file_path = "/tmp/qarpediem-datapoller.pid";
	ifstream pid_file(pid_file_path, ios::in);
	getline(pid_file, pid_str);

	return (pid_t) stoll(pid_str);
}

int main(int argc, char* argv[]){
	cout << "Qarpediem Data Sender" << endl;

	string db_path = "";
	if(argc > 1){
		db_path = argv[1];
	}

	cout << "opening database " << db_path << endl;
	database = new SensorsDatabase(db_path, false);

	curl_global_init(CURL_GLOBAL_DEFAULT);
	
	string mode;
	while(main_loop){
		mode = database->get_config("send_mode");
		try {
			if(mode == "ip"){
				runtime_ip();
			}
			else if(mode == "lorawan"){
				runtime_lorawan();
			} 
			else {
				throw runtime_error(mode + " is not a supported send mode");
			}
		} catch(std::exception& e){
			cerr << "error: " << endl << e.what() << endl;
		}

		//kill(get_poller_pid(), SIGUSR1);
		sleep(1);
	}

	cout << "closing database" << endl;
	delete database;

	return 0;
}

