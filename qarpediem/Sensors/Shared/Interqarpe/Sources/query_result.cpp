#include <InterQarpe.h>
#include <QueryResult.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <ctime>

using namespace InterQarpe;

QueryResult::
QueryResult(QueryResult::result_t result, uint8_t* data, size_t data_size){

	query_result = result;
	this->data_size = data_size;

	if(data != NULL){
		query_data = data;
	}
}

QueryResult::result_t QueryResult::get_result(void){
	return query_result;
}
