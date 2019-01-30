#include "QueryResult.h"

namespace InterQarpe {

/*
template<typename T>
bool QueryResult::get_data(T* data){
	T* buffer = (T*) query_data;
	if(sizeof(T) == data_size){
		*data = *buffer;
		return true;
	} else {
		return false;
	}
}

};

*/

template<typename T>
bool QueryResult::get_data(T* data){
	T* buffer = (T*) query_data;
	*data = *buffer;
	if((sizeof(T) *2)== data_size){ // *2 because the address of sensor
		return true;
	} else {
		return false;
	}
}

};
