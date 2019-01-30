#ifndef __MULTIPLEXER__
#define __MULTIPLEXER__

#include <Arduino.h>

class Mux8 {
	int pin_select[3];
public:
	typedef enum {
		S1 = 0,
		S2 = 1,
		S3 = 2,
		S4 = 3,
		S5 = 4,
		S6 = 5,
		S7 = 6,
		S8 = 7
	} output_t;

	/**
	 * Constructor of Mux8
	 * @param  A0 1st output selection pin
	 * @param  A1 2nd output selection pin
	 * @param  A2 3rd output selection pin
	 */
	Mux8(int A0, int A1, int A2);

	/**
	 * Select output
	 * @param enforcing output as the output of the mux
	 */
	void select_output(output_t output);
};

#endif // __MULTIPLEXER__
