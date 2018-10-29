#ifndef __INTERQARPE_DUPLEXBASE_TPP__
#define __INTERQARPE_DUPLEXBASE_TPP__

#include "QueryResult.h"
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

using namespace InterQarpe;

template<typename T>
void DuplexBase::send_response_ok(T data){
	write_packet(PAQ_RESPONSE_OK, (uint8_t*) &data, sizeof(T));
}

template<typename T>
void DuplexBase::send_response_error(T data){
	write_packet(PAQ_RESPONSE_ERROR, (uint8_t*) &data, sizeof(T));
}

#endif // __INTERQARPE_DUPLEXBASE_TPP__
