#ifndef __INTERQARPE_DUPLEXBASE_TPP__
#define __INTERQARPE_DUPLEXBASE_TPP__

#include "QueryResult.h"
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <Arduino.h>

using namespace InterQarpe;

template<typename T>
void DuplexBase::send_response_ok(T data){
	int adr=getAddress();
	  float dt[2];
			 dt[0]=(float)data;
			 dt[1]=(float)adr;
			Serial.print("Address = ");
			Serial.println(adr);
			size_t zz=sizeof(T)*2;
			write_packet(PAQ_RESPONSE_OK, (uint8_t*) dt, zz);
	//write_packet(PAQ_RESPONSE_OK,(uint8_t *)&data, sizeof(T));
}

template<typename T>
void DuplexBase::send_response_error(T data){
	write_packet(PAQ_RESPONSE_ERROR, (uint8_t*) &data, sizeof(T));
}


template<typename T>
void DuplexBase::addAddress_to_data_packet(T *data,int addr){

	/*T *dt=new T[2]; surtout pas de new: probleme allocation dynamique
			dt[0]=data;
			dt[1]=(T)addr;


        T * ptr=data + 1;
        *ptr=(T)addr;*/
}

#endif // __INTERQARPE_DUPLEXBASE_TPP__
