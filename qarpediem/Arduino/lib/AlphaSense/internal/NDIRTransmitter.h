#ifndef __ALPHASENSE_DRIVER_NDIR_TRANSMITTER__
#define __ALPHASENSE_DRIVER_NDIR_TRANSMITTER__

#include <usbhub.h>
#include <cdcftdi.h>

namespace AlphaSense {

class FTDIAsync : public FTDIAsyncOper {
public:
	uint8_t OnInit(FTDI* ftdi);
};

class NDIRTransmitter {
	USB* usb;
	FTDI* ftdi;
	FTDIAsync async;

public:
	/**
	 * Constructor of NDIRTransmitter
	 *
	 * @method NDIRTransmitter
	 * @param usb USB driver
	 * @param ftdi FTDI instance associated with the TXBoard
	 * @param
	 */
	NDIRTransmitter(USB* usb, FTDI* ftdi) : usb(usb), ftdi(ftdi) {}

	/**
	 * Return a value from the Transmitter Board
	 * @method get_value
	 * @param  value Value returned by the board
	 * @return true if we succesfully returned data
	 */
	bool get_value(float& value);
};

}

#endif // __ALPHASENSE_DRIVER_NDIR_TRANSMITTER__
