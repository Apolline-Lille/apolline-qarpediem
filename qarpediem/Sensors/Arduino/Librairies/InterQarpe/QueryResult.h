#ifndef __INTERQARPE_QUERYRESULT__
#define __INTERQARPE_QUERYRESULT__

#include <stdint.h>
#include <stddef.h>

namespace InterQarpe {

/**
 * Represent the result of a query
 * @tparam T represent the data the query should
 **/
class QueryResult {
public:
	/**
	 * Different results a query can take:
	 * - RESPONSE_[OK|ERROR]: Positive/negative response from the other device.
	 * - BAD_QUERY: Error received from the other device.
	 * - TIMEOUT: No response from the other device.
	 **/
	typedef enum {
		RESPONSE_OK,
		RESPONSE_ERROR,
		BAD_QUERY,
		TIMEOUT
	} result_t;

	/**
	 * Constructor of QueryResult
	 *
	 * @param result of the query
	 * @param data a pointer to the data
	 * @param data_size number of bytes available in the data buffer
	 **/
	QueryResult(result_t status, uint8_t* data, size_t data_size);

	/**
	 * Return the current status of the query
	 *
	 * @return result of query
	 */
	result_t get_result(void);

	/**
	 * Return data received from the other device.
	 *
	 * @param data will be returned through this pointer if any
	 * @tparam T type of data that will be returned
	 *
	 * @return true if we were able do recover enough data to build a T
	 */
	template<typename T>
	bool get_data(T* data);

private:
#ifndef __DOXYGEN__
	uint8_t* query_data;
	result_t query_result;
	size_t data_size;
#endif // __DOXYGEN__
};

#include "QueryResult.tpp"

};


#endif // __INTERQARPE_QUERYRESULT__
