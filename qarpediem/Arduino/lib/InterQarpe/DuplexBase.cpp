#include "DuplexBase.h"
#include <Arduino.h>
#include "QueryResult.h"
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

DuplexBase::DuplexBase() {
	last_sent_heartbeat = 0;
	last_received_heartbeat = 0;
	connected = false;
	packet.data_size = 0;
	incoming_query = false;
}

bool DuplexBase::timeout(uint32_t start, uint32_t timeout_ms){
	return (now_ms() - start) > timeout_ms;
}

uint8_t DuplexBase::compute_checkcode(uint8_t* buffer, size_t buffer_size){
	uint8_t checkcode = 0;
	for(size_t i = 0; i < buffer_size; i++){
		checkcode ^= buffer[i];
	}
	return checkcode;
}

bool DuplexBase::write_packet(uint8_t type, uint8_t* data, size_t data_length){
	size_t written_bytes = 0;
	uint8_t buffer[] = { PREFIX1, PREFIX2, type, (uint8_t) data_length, 0 };
	uint8_t data_checkcode = compute_checkcode(data, data_length);
	buffer[4] = compute_checkcode(buffer, sizeof(buffer));

	written_bytes += write_bytes(buffer, sizeof(buffer));
	written_bytes += write_bytes(data, data_length);
	written_bytes += write_bytes(&data_checkcode, 1);
	return written_bytes == (sizeof(buffer) + data_length + 1);
}

void DuplexBase::handle_packet_reception(void){
	if(packet_waiting() && parse_packet_header() && retrieve_packet_data()){
		packet.available = true;
	}
}

int DuplexBase::read_byte(void){
	uint8_t byte;
	if(read_bytes(&byte, 1) == 1){
		return (int) byte;
	} else {
		return -1;
	}
}

bool DuplexBase::packet_waiting(void){
	while(bytes_available() >= SIZEMIN_PACKET){
		if(read_byte() == (int) PREFIX1 && read_byte() == (int) PREFIX2){
			return true;
		}
	}
	return false;
}

bool DuplexBase::parse_packet_header(void){
	uint8_t header[SIZE_HEADER];
	uint8_t checkcode;
	uint8_t received_checkcode;

	if(read_bytes(header, SIZE_HEADER) == SIZE_HEADER){
		received_checkcode = header[2];
		checkcode = compute_checkcode(header, SIZE_HEADER-1);
		checkcode = (PREFIX1 ^ PREFIX2) ^ checkcode;
		if(checkcode == received_checkcode){
			packet.type = header[0];
			packet.data_size = (size_t) header[1];

			return true;
		}
	}

	return false;
}

bool DuplexBase::retrieve_packet_data(void){
	if(packet.data_size == 0){
		return true;
	}

	uint32_t start = now_ms();
	// We wait for the data AND the checkcode at the end
	while(bytes_available() <= packet.data_size){
		if(timeout(start, TIMEOUT_DATA)){
			return false;
		}
	}

	if(read_bytes(packet.data, packet.data_size) == (int) packet.data_size){
		uint8_t received_code = (uint8_t) read_byte();
		uint8_t code = compute_checkcode(packet.data, packet.data_size);
		return code == received_code;
	}

	return true;
}

bool DuplexBase::wait_packet_or_timeout(uint8_t type, uint32_t tm_ms){
	uint32_t start_time = now_ms();
	while(!timeout(start_time, tm_ms)){
		routine();
		if(packet.available && packet.type == type){
			return true;
		}
	}
	return false;
}

bool DuplexBase::wait_query_response_or_timeout(){
	uint32_t start = now_ms();
	while(!timeout(start, TIMEOUT_QUERY_RESPONSE)){
		routine();
		if(packet.available && (packet.type & MASK_RESPONSE) == MASK_RESPONSE){
			return true;
		}
	}
	return false;
}

bool DuplexBase::_query(const char* query){
	write_packet(PAQ_QUERY, (uint8_t*) query, strlen(query) + 1);

	if(!wait_packet_or_timeout(PAQ_QUERY_RECEIVED, TIMEOUT_QUERY_RECEIVED)){
		return false;
	}
	packet.available = false;

	if(!wait_query_response_or_timeout()){
		return false;
	}

	write_packet(PAQ_RESPONSE_RECEIVED, NULL, 0);

	return true;
}

QueryResult DuplexBase::query(const char* query){
	QueryResult::result_t result;
	if(!_query(query)){
		return QueryResult(QueryResult::TIMEOUT, NULL, 0);
	}

	switch(packet.type){
		case PAQ_RESPONSE_OK:
		result = QueryResult::RESPONSE_OK;
		break;

		case PAQ_RESPONSE_ERROR:
		result = QueryResult::RESPONSE_ERROR;
		break;

		case PAQ_RESPONSE_BADQUERY:
		result = QueryResult::BAD_QUERY;
		break;
	}

	packet.available = false;
	return QueryResult(result, packet.data, packet.data_size);
}

void DuplexBase::handle_incoming_query(){
	if(packet.available && packet.type == PAQ_QUERY){
		packet.available = false;
		if(packet.data[packet.data_size - 1] != 0){
			return;
		}

		write_packet(PAQ_QUERY_RECEIVED, NULL, 0);

		incoming_query = true;
		on_query((const char*) packet.data);
		incoming_query = false;
	}
}

void DuplexBase::send_response_ok(void){
	write_packet(PAQ_RESPONSE_OK, NULL, 0);
}

void DuplexBase::send_response_error(void){
	write_packet(PAQ_RESPONSE_ERROR, NULL, 0);
}

void DuplexBase::send_badquery(void){
	size_t try_count = 0;
	while(try_count++ < 3){
		write_packet(PAQ_RESPONSE_BADQUERY, NULL, 0);
		if(wait_packet_or_timeout(PAQ_RESPONSE_RECEIVED, TIMEOUT_RESPONSE_RECEIVED)){
			return;
		}
	}
}

void DuplexBase::on_query(const char* query){
	send_badquery();
}

void DuplexBase::handle_connection(){
	if(timeout(last_sent_heartbeat, TIMEOUT_SEND_HEARTBEAT)){
		write_packet(PAQ_HEARTBEAT, NULL, 0);
		last_sent_heartbeat = now_ms();
	}

	if(packet.available && packet.type == PAQ_HEARTBEAT){
		last_received_heartbeat = now_ms();
		packet.available = false;
		Serial.print("packet heartb ok ");
		connected = true;
	}

	if(timeout(last_received_heartbeat, TIMEOUT_CONNECTION_LOST)){
		//Serial.print("time lost");
		connected = false;
	}
}

void DuplexBase::routine(void){
	handle_packet_reception();
	handle_connection();

	if(!incoming_query){
		handle_incoming_query();
	}
}

bool DuplexBase::is_connected(void){
	return connected;
}


int DuplexBase::getSensorAddress(String sensorName){
    int adr;

    if(sensorName=="motion"){
        adr=MOTION;
    }else if (sensorName=="sound") {
        adr=SOUND;
    }else if (sensorName=="luminosity") {
        adr=LUMINOSITY;
    }else if (sensorName=="humidity") {
        adr=HUMIDITY;
    }else if (sensorName=="pressure") {
        adr=PRESSURE;
    }else if (sensorName=="temperature") {
        adr=TEMPERATURE;
    }else if (sensorName=="aps1") {
        adr=APS1;
    }else if (sensorName=="aps2") {
        adr=APS2;
    }else if (sensorName=="aps3") {
        adr=APS3;
    }else if (sensorName=="aps4") {
        adr=APS4;
    }else if (sensorName=="aps5") {
        adr=APS5;
    }else if (sensorName=="aps6") {
        adr=APS6;
    }else if (sensorName=="aps7") {
        adr=APS7;
    }else if (sensorName=="aps8") {
        adr=APS8;
    }else if (sensorName=="co2") {
        adr=CO2;
    }else if (sensorName=="dust_pm1") {
        adr=DUST_PM1;
    }else if (sensorName=="dust_pm2.5") {
        adr=DUST_PM25;
    }else if (sensorName=="dust_pm10") {
        adr=DUST_PM10;
    }else if (sensorName=="dust_bin00_partcc") {
        adr=DUST_BIN00_PARTCC;
    }else if (sensorName=="dust_bin01_partcc") {
        adr=DUST_BIN01_PARTCC;
    }else if (sensorName=="dust_bin02_partcc") {
        adr=DUST_BIN02_PARTCC;
    }else if (sensorName=="dust_bin03_partcc") {
        adr=DUST_BIN03_PARTCC;
    }else if (sensorName=="dust_bin04_partcc") {
        adr=DUST_BIN04_PARTCC;
    }else if (sensorName=="dust_bin05_partcc") {
        adr=DUST_BIN05_PARTCC;
    }else if (sensorName=="dust_bin06_partcc") {
        adr=DUST_BIN06_PARTCC;
    }else if (sensorName=="dust_bin07_partcc") {
        adr=DUST_BIN07_PARTCC;
    }else if (sensorName=="dust_bin08_partcc") {
        adr=DUST_BIN08_PARTCC;
    }else if (sensorName=="dust_bin09_partcc") {
        adr=DUST_BIN09_PARTCC;
    }else if (sensorName=="dust_bin10_partcc") {
        adr=DUST_BIN10_PARTCC;
    }else if (sensorName=="dust_bin11_partcc") {
        adr=DUST_BIN11_PARTCC;
    }else if (sensorName=="dust_bin12_partcc") {
        adr=DUST_BIN12_PARTCC;
    }else if (sensorName=="dust_bin13_partcc") {
        adr=DUST_BIN13_PARTCC;
    }else if (sensorName=="dust_bin14_partcc") {
        adr=DUST_BIN14_PARTCC;
    }else if (sensorName=="dust_bin15_partcc") {
        adr=DUST_BIN15_PARTCC;
    }else {
        adr=0;
    }


    return adr;

}
