#ifndef __INTERQARPE__
#define __INTERQARPE__

#include <stdint.h>
#include <stddef.h>
#include "QueryResult.h"

namespace InterQarpe {

class DuplexBase {
#ifndef __DOXYGEN__
	// All packet types and their values in the "packet type" field
	static const uint8_t PAQ_HEARTBEAT = 0x0;
	static const uint8_t PAQ_QUERY = 0x10;
	static const uint8_t PAQ_QUERY_RECEIVED = 0x11;
	static const uint8_t PAQ_RESPONSE_RECEIVED = 0x12;
	static const uint8_t PAQ_RESPONSE_OK = 0x21;
	static const uint8_t PAQ_RESPONSE_ERROR = 0x22;
	static const uint8_t PAQ_RESPONSE_BADQUERY = 0x23;
	static const uint8_t PAQ_PROTOCOL_VERSION = 0xFF;

	/// Mask to check if we received a response (all response types are between
	/// 0x21 and 0x2F included)
	static const uint8_t MASK_RESPONSE = 0x20;

	// Default max duration in milliseconds before we consider we've timeout
	static const uint32_t TIMEOUT_DATA = 50;
	static const uint32_t TIMEOUT_QUERY_RECEIVED = 200;
	static const uint32_t TIMEOUT_RESPONSE_RECEIVED = 200;
	static const uint32_t TIMEOUT_QUERY_RESPONSE = 2000;
	static const uint32_t TIMEOUT_CONNECTION_LOST = 5000;
	static const uint32_t TIMEOUT_SEND_HEARTBEAT = 2000;

	// Number of try if we don't receive a PAQ_[QUERY|RESPONSE]_RECEIVED
	static const size_t TRY_SEND_QUERY = 3;
	static const size_t TRY_SEND_RESPONSE = 3;

	// Packet sections size
	static const size_t SIZE_PREFIX = 2;
	/// Header size excluding prefix
	static const size_t SIZE_HEADER = 3;
	/// excluding the checkcode at the end
	static const size_t SIZEMAX_DATA = 255;
	/// We consider a packet with no data
	static const size_t SIZEMIN_PACKET = SIZE_HEADER + SIZE_PREFIX;

	// Prefix value of the InterQarpe packet
	static const uint8_t PREFIX1 = 0xB1;
	static const uint8_t PREFIX2 = 0X42;
	
	/**
	 * Represent a packet we've received.
	 **/
	typedef struct {
		bool available;
		uint8_t type;
		size_t data_size;
		uint8_t data[SIZEMAX_DATA];
		uint32_t __force_align;
	} packet_t;

	/**
	 * Represent the last packet to arrived.
	 * Because InterQarpe is a "synchronous" protocol (we don't support sending
	 * multiple queries at once and wait for multiple responses. we are in a
	 * "one query => one reponse" model), if the packet is not consumed, we will
	 * wait until it is to read the next packet.
	 **/
	packet_t packet;

	/**
	 * true if we have a valid connection with the other device
	 **/
	bool connected;

	uint32_t last_received_heartbeat;
	uint32_t last_sent_heartbeat;
	bool incoming_query;

	/**
	 * Write a packet on the serial line
	 *
	 * @param type the type of the packet to be send (type field). The value
	 * must be one of the PAQ_* constant. Otherwise, behavior is undefined.
	 * @param data a pointer to "data_length" bytes to be sent in the package
	 * @param data_length number of bytes to send in "data". Max data length
	 * is 255 as defined by the InterQarpe protocol.
	 *
	 * @return true if it was succesfully sent
	 **/
	bool write_packet(uint8_t type, uint8_t* data, size_t data_length);

	/**
	 * Recover next packet on the serial line
	 **/
	void handle_packet_reception(void);

	/**
	 * Read one byte from the serial port. It will not wait for a byte to
	 * arrived if we don't have any waiting on the serial port.
	 *
	 * @return the value of the byte read from the serial port or -1
	 **/
	int read_byte(void);

	/**
	 * Check if a packet is waiting on the serial port
	 *
	 * @return true if a packet is waiting (will consume the 2-bytes prefix)
	 **/
	bool packet_waiting(void);

	/**
	 * Parse the incoming header waiting on the serial port and check if
	 * it was not corrupted.
	 *
	 * @return true if a packet header has been pased succesfully 
	 **/
	bool parse_packet_header(void);

	/**
	 * Retrieve incoming data frame from the serial port
	 *
	 * @return true if we have succesfully retrieve data
	 **/
	bool retrieve_packet_data(void);

	/**
	 * Compute the checkcode of the data in "buffer"
	 * The checkcode is just a xor of all the value in the buffer.
	 * checkcode([5, 4, 7]) = (((0 xor 5) xor 4) xor 7)
	 *
	 * @param buffer the buffer that contains the data
	 * @param buffer_size size of buffer
	 * 
	 * @return xor checkcode of the data in buffer
	 **/
	uint8_t compute_checkcode(uint8_t* buffer, size_t buffer_size);

	/**
	 * Check if we have timeout
	 *
	 * @param start 
	 * @param timeout_ms
	 *
	 * @return true if we have timeout, false if we still have time
	 **/
	bool timeout(uint32_t start, uint32_t timeout_ms);

	/**
	 * Wait for a packet of a certain type to arrive or timeout
	 *
	 * @param type type of the packet (must be on of the PAQ_* constant)
	 * @param tm_ms time before we considered we've timeout in milliseconds
	 *
	 * @return true if we've received a packet of the type given before we've
	 * timeout. false otherwise.
	 **/
	bool wait_packet_or_timeout(uint8_t type, uint32_t tm_ms);

	/**
	 * Wait of a response packet to arrive or timeout
	 *
	 * @return true if we've received a query response before we've timeout
	 **/
	bool wait_query_response_or_timeout();

	/**
	 * Initiate a query
	 *
	 * @param query the query that will be sent
	 *
	 * @return true if we succesfully initiated a query
	 **/
	bool initiate_query(const char* query);

	/**
	 * Internal function to send a query
	 * (cf query() for more documentation)
	 **/
	bool _query(const char* query);

	/**
	 * Handle any query coming from the other device
	 **/
	void handle_incoming_query();

	/**
	 * Manage heartbeat sending and receiving and connection status
	 **/
	void handle_connection();

#endif // __DOXYGEN__

	/**
	 * Write bytes in the serial port
	 * 
	 * @param buf pointer to "count" bytes that will be send through the
	 * serial port
	 * @param count number of bytes to be sent
	 * 
	 * @return number of bytes written succesfully or -1 if we failed to write
	 * altogether and exception are disabled
	 **/
	virtual int write_bytes(uint8_t* buf, size_t count) = 0;

	/**
	 * Read "count" bytes from the serial port. If less bytes are available
	 * this function will not wait for the missing bytes and return
	 * 
	 * @param buf pointer to a buffer of "count" bytes that will be
	 * overwritten
	 * @param count number of bytes to read from the serial port
	 * 
	 * @return number of bytes read successfully from the serial port
	 * (can be less than "count") or -1 if an error occur and exception are
	 * disabled.
	 **/
	virtual int read_bytes(uint8_t* buf, size_t count) = 0;

	/**
	 * Return the number of bytes available on the serial port
	 * 
	 * @return number of bytes available (if we can't get the value because
	 * of an error, we should return 0)
	 **/
	virtual size_t bytes_available(void) = 0;

	/**
	 * Return the time in milliseconds (used for timeout, so we don't care
	 * of the absolute value)
	 * 
	 * @return return the current time
	 **/
	virtual uint32_t now_ms(void) = 0;

protected:
	/**
	 * send a "ok" response to the other device
	 *
	 * @tparam T type of data to send
	 **/
	template<typename T>
	void send_response_ok(T* data);

	/**
	 * send a "error" response to the other device
	 *
	 * @tparam T type of data to send
	 **/
	template<typename T>
	void send_response_error(T* data);

	/**
	 * Signal to the other side that the query is not handled on our side
	 **/
	void send_badquery(void);

	/**
	 * Hook that is called when a query is received
	 **/
	virtual void on_query(const char* query);

public:
	DuplexBase();

	/**
	 * Execute common task needed by the InterQarpe protocol to manage
	 * the connection. This function must be executed in order to receive
	 * packet. You may want to execute it in each iteration of your main
	 * loop.
	 **/
	void routine(void);

	/**
	 * Check if we have we are connected to the other device
	 * 
	 * @return true if we have a valid connection with the other device
	 **/
	bool is_connected(void);

	/**
	 * Send a query to the other device. This function is blocking and will
	 * only return when we have a proper response to return
	 * (a.k.a response from the other device or timeout after the default
	 * duration without a response)
	 *
	 * @param query the query must be a 0-terminated string
	 *
	 * @return the query's result 
	 **/
	QueryResult query(const char* query);
};

}

#include "InterQarpe.tpp"

#endif // __INTERQARPE__

