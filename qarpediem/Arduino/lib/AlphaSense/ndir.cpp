#include "internal/NDIRTransmitter.h"
#include <usbhub.h>
#include <cdcftdi.h>

using namespace AlphaSense;

namespace AlphaSense {
	FTDIAsync NDIRTransmitterAsync;
};

uint8_t FTDIAsync::OnInit(FTDI* ftdi){
	Serial.println("OnInit");
	uint8_t rc = ftdi->SetBaudRate(19200);
	if(rc){
		return rc;
	}

	rc = ftdi->SetFlowControl(FTDI_SIO_DISABLE_FLOW_CTRL);
	if(rc){
		return rc;
	}

	return rc;
}

bool NDIRTransmitter::get_value(float& value){
	if(usb->getUsbTaskState() == USB_STATE_RUNNING){
		Serial.println("pulling");
		char query[] = "N\r";

		if(ftdi->SndData(2, (uint8_t*) query)){
			return false;
		}

		uint8_t buffer[65];
		uint16_t received_size = 64;
		memset(buffer, 0, sizeof(buffer));

		if(ftdi->RcvData(&received_size, buffer)){
			return false;
		}

		if(received_size > 2){
			String data((char*)(buffer + 2));
			value = data.toFloat();
			return true;
		}
	}

	return false;
}
