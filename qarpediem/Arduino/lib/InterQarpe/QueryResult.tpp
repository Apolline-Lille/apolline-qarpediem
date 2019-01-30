#ifndef __INTERQARPE_QUERYRESULT_TPP__
#define __INTERQARPE_QUERYRESULT_TPP__

#include "QueryResult.h"

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

#endif // __INTERQARPE_QUERYRESULT_TPP__
