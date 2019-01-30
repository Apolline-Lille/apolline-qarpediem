#include "InterQarpe.h"

using namespace InterQarpe;

template<typename T>
void DuplexBase::send_response_ok(T* data){
	write_packet(PAQ_RESPONSE_OK, (uint8_t*) data, sizeof(T));
}

template<typename T>
void DuplexBase::send_response_error(T* data){
	write_packet(PAQ_RESPONSE_ERROR, (uint8_t*) data, sizeof(T));
}


template<typename T>
void DuplexBase::addAddress_to_data_packet(T *data,size_t addr){
	T * pt=data + 1;
	*pt=(T)addr;


}


template<typename T> 
int DuplexBase::getRomteAddress(T *data){
    T *pt=data+1;
    int addr=(int)(*pt);

    return addr;
}
