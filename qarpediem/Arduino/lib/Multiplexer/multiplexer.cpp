#include "Multiplexer.h"

Mux8::Mux8(int A0, int A1, int A2){
	pin_select[0] = A0;
	pin_select[1] = A1;
	pin_select[2] = A2;

	digitalWrite(A0, LOW);
	digitalWrite(A1, LOW);
	digitalWrite(A2, LOW);
}

void Mux8::select_output(Mux8::output_t output_id){
	if(output_id & 1){
		digitalWrite(pin_select[0], HIGH);
	} else {
		digitalWrite(pin_select[0], LOW);
	}

	if(output_id & 2){
		digitalWrite(pin_select[1], HIGH);
	} else {
		digitalWrite(pin_select[1], LOW);
	}

	if(output_id & 4){
		digitalWrite(pin_select[2], HIGH);
	} else {
		digitalWrite(pin_select[2], LOW);
	}
}
