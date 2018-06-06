#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <ctime>
#include <stdint.h>
#include <system_error>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <InterQarpe.h>
#include <errno.h>
#include <SensorsDatabase.h>
#include <Sensors.h>

Sensors::Sensors() : port_fd(-1) {}

bool Sensors::open_port(string port){
	close_port();
	port_fd = open(port.c_str(), O_RDWR | O_NOCTTY | O_NDELAY | O_ASYNC);
	if(port_fd == -1){
		throw system_error(errno, system_category());
	}

	if(!configure_port(port_fd)){
		close_port();
		return false;
	}

	return true;
}

bool Sensors::close_port(void){
	bool status = close(port_fd) != -1;
	port_fd = -1;
	return status;
}

bool Sensors::is_port_open(void){
	return port_fd > -1;
}

int Sensors::write_bytes(uint8_t* buffer, size_t buffer_length){
	int count = write(port_fd, buffer, buffer_length);
	if(count == -1 && errno != EAGAIN && errno != EWOULDBLOCK){
		throw system_error(errno, system_category());
	}
	return count;
}

int Sensors::read_bytes(uint8_t *buffer, size_t buffer_length){
	int count = read(port_fd, buffer, buffer_length);
	if(count == -1 && errno != EAGAIN && errno != EWOULDBLOCK){
		throw system_error(errno, system_category());
	}
	return count;
}

size_t Sensors::bytes_available(void){
	size_t available = 0;
	if(ioctl(port_fd, FIONREAD, &available) == -1){
		throw system_error(errno, system_category());
	}
	return available;
}

uint32_t Sensors::now_ms(void){
	struct timespec spec;
	if(clock_gettime(CLOCK_MONOTONIC, &spec) != 0){
		throw system_error(errno, system_category());
	}
	return spec.tv_sec * 1000 + spec.tv_nsec / 1e6;
}

bool Sensors::configure_port(int fd){
	struct termios options;
	if(tcgetattr(fd, &options) == -1){
		throw system_error(errno, system_category());
	}

	options.c_cflag = B230400 | CS8 | CLOCAL | CREAD;
	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;

	if(tcsetattr(fd, TCSANOW, &options) == -1){
		throw system_error(errno, system_category());
	}

	fcntl(fd, F_SETFL, O_NDELAY | O_ASYNC);
	return true;
}
