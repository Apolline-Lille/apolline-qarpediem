#include <json/json.h>
#include <unordered_map>
#include <sstream>
#include <ctime>
#include <string>
#include <unistd.h>
#include <cstdint>
#include <curl/curl.h>

#include <SensorsDatabase.h>

using namespace std;

extern SensorsDatabase* database;

/**
 * Return the time given by the MONOTONIC clock.
 * check the manual for clock_gettime for more information
 **/
static time_t time_monotonic(){
	struct timespec v;
	clock_gettime(CLOCK_MONOTONIC, &v);
	return v.tv_sec;
}

/**
 * Convert json to text and send it using curl
 * Return true if the operation was succesfull
 **/
static bool send_json(Json::Value& polls){
	bool payload_sent = false;
	CURLcode res;
	long http_code;

	Json::FastWriter writer;
	string payload = writer.write(polls);

	cout << payload << endl;

	CURL* curl = curl_easy_init();
	string url = "http://" + database->get_config("server_host");
	url += ':' + database->get_config("server_port");
	url += "/airQualityJSON";
	struct curl_slist* headers = NULL;

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, payload.length());
	headers = curl_slist_append(headers, "Content-Type: application/json");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 60L);
	curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 30L);

	res = curl_easy_perform(curl);
	curl_slist_free_all(headers);

	cout << "request sent, waiting response !" << endl;

	if(res == CURLE_OK){
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
		if(http_code == 200){
			payload_sent = true;
			cout << "response received" << endl;
		} else {
			cerr << "failed to sent packet" << endl;
			cerr << "received" << http_code << " code" << endl;
		}
	} else {
		cerr << "failed to sent packet" << endl;
		cerr << curl_easy_strerror(res) << endl;
	}

	curl_easy_cleanup(curl);

	return payload_sent;
}

typedef SensorsDatabase::poll_t poll_t;

/**
 * send polls through the IP layer
 **/
static void send_polls(vector<time_t> polls){
	Json::Value root(Json::arrayValue);

	for(const time_t& poll_time : polls){
		Json::Value poll_json;
		poll_t poll = database->get_poll(poll_time);
		for(const auto& data : poll){
			poll_json[data.first] = data.second;
		}
		poll_json["date"] = ((uint64_t) poll_time) * 1000;
		poll_json["rssi"] = 255;
		poll_json["address"] = database->get_config("node_address");
		root.append(poll_json);
	}

	if(send_json(root)){
		for(const time_t& poll_time : polls){
			database->poll_sent(poll_time);
		}
	}		

}

/**
 * server_time curl callback
 **/
static size_t curl_on_data(char* ptr, size_t size, size_t nmemb, void* str){
	string* time_str = static_cast<string*>(str);
	size_t bytes_received = size * nmemb;
	for(size_t i = 0; i < bytes_received; i++){
		time_str->push_back(ptr[i]);
	}
	return bytes_received;
}

/**
 * Ask the server for the current time and return it
 * Return -1 if we weren't able to get it
 */
static time_t server_time(void){
	CURL* curl = curl_easy_init();
	CURLcode res;
	long http_code;

	time_t time = -1;
	string time_str = "";
	if(curl) {
		string url = "http://" + database->get_config("server_host");
		url += ':' + database->get_config("server_port");
		url += "/currentTime";

		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_on_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &time_str);
		curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 60L);
		curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 30L);

		res = curl_easy_perform(curl);

		if(res == CURLE_OK){
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
			if(http_code == 200){
				cout << "time_received: " << time_str << endl;
				return static_cast<time_t>(stoll(time_str) / 1000);
			}
		} else {
			cerr << "failed to sent packet" << endl;
			cerr << curl_easy_strerror(res) << endl;
		}
	}

	curl_easy_cleanup(curl);
	return time;
}

/**
 * Sync time by asking periodically the server for the current time
 * We could have used NTP but we're can't be sure if a time server is accessible
 * through the network.
 **/
static void sync_time(void){
	struct timespec new_time;
	static time_t last_sync = time_monotonic();
	static bool first_time = true;
	time_t now = time_monotonic();

	if((now - last_sync) > 3600 || first_time){
		new_time.tv_sec = server_time();
		new_time.tv_nsec = 0;
		if(new_time.tv_sec > -1){
			clock_settime(CLOCK_REALTIME, &new_time);
			last_sync = now;
			first_time = false;
		}
	}
}

/**
 * Send polls and sync time using the IP stack
 **/
void runtime_ip(void){
	sync_time();

	static time_t last_send = time_monotonic();
	time_t send_interval = stoll(database->get_config("interval_ip"));

	time_t now = time_monotonic();
	vector<time_t> polls;
	if(now - last_send >= send_interval){
		polls = database->get_sendable_polls(100);
		while(polls.size() > 0){
			send_polls(polls);
			polls = database->get_sendable_polls(100);
		}
		last_send = now;
	}
}

