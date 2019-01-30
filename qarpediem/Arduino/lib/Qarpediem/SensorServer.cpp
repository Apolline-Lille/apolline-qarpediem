#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <AD7193.h>
#include <DHT.h>
#include <Multiplexer.h>
#include <ChainableLED.h>
#include <AlphaSense.h>
#include <Digital_Light_TSL2561.h>
#include <SensorAverage.h>
#include <DuplexBase.h>
#include "SensorServer.h"
#include "Qarpediem.h"

using namespace Qarpediem;
using namespace InterQarpe;

USB usb;
FTDI ndir_ftdi(&usb, &AlphaSense::NDIRTransmitterAsync);
AlphaSense::NDIRTransmitter co2_sensor(&usb, &ndir_ftdi);

AlphaSense::OPCN2 dust_sensor;

AD7193 adc(Qarpediem::SS_CAN);
Mux8 mux_com(Qarpediem::MUX_PIN1, Qarpediem::MUX_PIN2, Qarpediem::MUX_PIN3);

DHT dht(Qarpediem::DIG_TH, DHT22);

SensorAverage motion_sensor;
SensorAverage sound_sensor;

ChainableLED status_led(27, 25, 1);

SensorServer::SensorServer(HardwareSerial* serial)
	: serial(serial)
	, last_poll_average(0)
	, last_ping_dust(0) {}

void SensorServer::init(void){
	status_led.init();
	status_led.setColorRGB(0, 0, 0, 50);

	serial->begin(230400);

	delay(2000);

	Serial.println("configuring SPI...");
	SPI.begin();

	digitalWrite(Qarpediem::SS_DUST, HIGH);
	digitalWrite(Qarpediem::SS_CAN, HIGH);

	Serial.println("configuring ADC...");
	if(adc.is_alive()){
		configure_adc();
	} else {
		Serial.println("> not responding");
	}

	Serial.println("configuring I2C...");
	Wire.begin();

	Serial.println("configuring TSL2561 (Temperature & Humidity sensor)...");
	TSL2561.init();

	Serial.println("configuring usb interface...");
	if(usb.Init() == -1){
		Serial.println(" > error while configuring usb interface");
	}

	Serial.println("configuring OPCN2 (dust sensor)...");
	delay(1000);
	dust_sensor.begin(Qarpediem::SS_DUST);
	dust_sensor.on();
	delay(8000);
	if(!dust_sensor.ping()){
		Serial.println("> can't contact the dust sensor");
	}

	dust_histogram = dust_sensor.read_histogram();


	last_histogram_dust = now_ms();

	Serial.println("Configuring pin");
	pinMode(Qarpediem::ANALOG_SON_ENVOLOPE, INPUT);
	pinMode(Qarpediem::ANALOG_PRESSION, INPUT);
	pinMode(Qarpediem::DIG_MVT, INPUT);

	Serial.flush();

}


void SensorServer::routine(void){
	DuplexBase::routine();

	float sampling_period = motion_sensor.get_sampling_period() * 1000;
	uint32_t now = now_ms();
	if(now - last_poll_average >= (uint32_t) sampling_period){
		last_poll_average = now;
		motion_sensor.push_value(digitalRead(Qarpediem::DIG_MVT));
		sound_sensor.push_value(analogRead(Qarpediem::ANALOG_SON_ENVOLOPE));
	}

//	status_led.setColorRGB(0, 0, 30, 0);// ALpha. à suprimer apres
	if(is_connected()){ // à decommenter
		status_led.setColorRGB(0, 0, 30, 0);
	} else {
		status_led.setColorRGB(0, 30, 0, 0);
	}

	if(now_ms() - last_histogram_dust > 5000){
		last_histogram_dust = now_ms();
		dust_histogram = dust_sensor.read_histogram();

	}

	usb.Task();
}

int SensorServer::write_bytes(uint8_t *buffer, size_t buffer_length){
	return serial->write(buffer, buffer_length);
};

int SensorServer::read_bytes(uint8_t *buffer, size_t buffer_length){
	return serial->readBytes(buffer, buffer_length);
}

size_t SensorServer::bytes_available(){
	return serial->available();
}

uint32_t SensorServer::now_ms(){
	return millis();
}

void SensorServer::configure_adc(){
	adc.set_reference_voltage(5000);
	adc.set_chop(true);
	adc.set_difftype(AD7193::DIFF_PSEUDO);
	adc.set_gain(AD7193::GAIN1);
	adc.set_reference(AD7193::REFIN1);
	adc.set_polarity(AD7193::BIPOLAR);
	adc.set_rej60(false);
}

void SensorServer::on_sensor_adc(char id){
	adc.disable_all_channels();

	switch (id) {
		case '1':
		mux_com.select_output(Mux8::S1);
		adc.enable_channel(AD7193::CHAN0);
		break;

		case '2':
		mux_com.select_output(Mux8::S2);
		adc.enable_channel(AD7193::CHAN1);
		break;

		case '3':
		mux_com.select_output(Mux8::S3);
		adc.enable_channel(AD7193::CHAN2);
		break;

		case '4':
		mux_com.select_output(Mux8::S5);
		adc.enable_channel(AD7193::CHAN4);
		break;

		case '5':
		mux_com.select_output(Mux8::S6);
		adc.enable_channel(AD7193::CHAN5);
		break;

		case '6':
		mux_com.select_output(Mux8::S7);
		adc.enable_channel(AD7193::CHAN6);
		break;

		case '7':
		mux_com.select_output(Mux8::S4);
		adc.enable_channel(AD7193::CHAN3);
		break;

		case '8':
		mux_com.select_output(Mux8::S8);
		adc.enable_channel(AD7193::CHAN7);
		break;

		default:
		return send_badquery();
	}

	float reading = adc.reading(NULL);
	send_response_ok(reading);
}

void SensorServer::on_sensor_dust(String sensor_name){
	if(sensor_name == "dust_pm1"){
		send_response_ok((float) dust_histogram.pm1);
	} else if(sensor_name == "dust_pm2.5"){
		send_response_ok((float) dust_histogram.pm25);
	} else if(sensor_name == "dust_pm10"){
		send_response_ok((float) dust_histogram.pm10);
	} else if(sensor_name == "dust_histogram"){
		send_response_error();
		Serial.println("not implemented ");
	} else {
		send_dust_bin_particleCC(sensor_name);
	}
}

void SensorServer::send_dust_bin_particleCC(String sensor_name){
	if (sensor_name=="dust_bin00_partcc") {
			send_response_ok((float) dust_histogram.bin0);
	}else if (sensor_name=="dust_bin01_partcc") {
			send_response_ok((float) dust_histogram.bin1);
	}else if (sensor_name=="dust_bin02_partcc") {
			send_response_ok((float) dust_histogram.bin2);
	}else if (sensor_name=="dust_bin03_partcc") {
			send_response_ok((float) dust_histogram.bin3);
	}else if (sensor_name=="dust_bin04_partcc") {
			send_response_ok((float) dust_histogram.bin4);
	}else if (sensor_name=="dust_bin05_partcc") {
			send_response_ok((float) dust_histogram.bin5);
	}else if (sensor_name=="dust_bin06_partcc") {
			send_response_ok((float) dust_histogram.bin6);
	}else if (sensor_name=="dust_bin07_partcc") {
			send_response_ok((float)dust_histogram.bin7);
	}else if (sensor_name=="dust_bin08_partcc") {
			send_response_ok((float) dust_histogram.bin8);
	}else if (sensor_name=="dust_bin09_partcc") {
			send_response_ok((float) dust_histogram.bin9);
	}else if (sensor_name=="dust_bin10_partcc") {
			send_response_ok((float) dust_histogram.bin10);
	}else if (sensor_name=="dust_bin11_partcc") {
			send_response_ok((float) dust_histogram.bin11);
	}else if (sensor_name=="dust_bin12_partcc") {
			send_response_ok((float) dust_histogram.bin12);
	}else if (sensor_name=="dust_bin13_partcc") {
			send_response_ok((float) dust_histogram.bin13);
	}else if (sensor_name=="dust_bin14_partcc") {
			send_response_ok((float) dust_histogram.bin14);
	}else if (sensor_name=="dust_bin15_partcc") {
			send_response_ok((float) dust_histogram.bin15);
	}else{
			send_badquery();
		}

}

void SensorServer::on_sensor(String sensor_name){
		String tmpname=sensor_name;
		address=getSensorAddress(tmpname);

	if(sensor_name.startsWith("aps") && sensor_name.length() == 4){
		if(adc.is_alive()){
			on_sensor_adc(sensor_name.charAt(3));
		} else {
			send_response_error();
		}
	}else if(sensor_name.startsWith("dust_")){
		if(dust_sensor.ping()){
			on_sensor_dust(sensor_name);
		} else {
			send_response_error();
		}
	} else if(sensor_name == "co2"){
		float value;
		if(co2_sensor.get_value(value)){
			send_response_ok(value);
		} else {
			send_response_error();
		}
	} else if(sensor_name == "temperature"){
		float temperature = dht.readTemperature();
		if(!isnan(temperature)){
			send_response_ok<float>(temperature);
		} else {
			send_response_error();
		}
	} else if(sensor_name == "humidity"){
		float humidity = dht.readHumidity();
		if(!isnan(humidity)){
			send_response_ok(humidity);
		} else {
			send_response_error();
		}
	} else if(sensor_name == "luminosity"){

		float lux =  TSL2561.readVisibleLux(); //lecture qui bloque.
		if(lux != -1){
			send_response_ok(lux);
		} else {
			send_response_error();
		}

	} else if(sensor_name == "motion"){
		send_response_ok(motion_sensor.get_average_level() * 100.0);
	} else if(sensor_name == "sound"){
		send_response_ok(sound_sensor.get_average_level() / 1024.0 * 5000.0);
	} else if(sensor_name == "pressure"){
		float pressure = (float) analogRead(Qarpediem::ANALOG_PRESSION);
		send_response_ok(pressure / 1024.0 * 5000.0);
	} else {
		send_badquery();
	}
}

void SensorServer::on_config(String config){
	if(config.startsWith("readings::motion")){
		send_response_error();
	} else if(config.startsWith("readings::sound")){
		send_response_error();
	} else {
		send_badquery();
	}
}

void SensorServer::on_query(const char *query_str){
	Serial.print("\n incoming query: ");
	Serial.println(query_str);

	String query(query_str);
	status_led.setColorRGB(0, 20, 40, 82);
	String sensor = "sensors::";
	String config = "config::";

	if(query.startsWith(sensor)){
		on_sensor(query.substring(sensor.length()));
	} else if(query.startsWith(config)){
		on_config(query.substring(config.length()));
	} else {
		send_badquery();
	}
}



void SensorServer::testeOPCN()
{
	if(now_ms() - last_histogram_dust > 5000){
		last_histogram_dust = now_ms();
		dust_histogram = dust_sensor.read_histogram();

	Serial.println("-------------------------------------------");
	Serial.print("dust_pm1 :");
	Serial.println((float)dust_histogram.pm1);
	Serial.print("dust_pm25 :");
	Serial.println((float)dust_histogram.pm25);
	Serial.print("dust_pm10 :");
	Serial.println((float)dust_histogram.pm10);
	Serial.print("dust_bin0 :");
	Serial.println((float)dust_histogram.bin0);
	Serial.print("dust_bin1 :");
	Serial.println((float)dust_histogram.bin1);
	Serial.print("dust_bin2 :");
	Serial.println((float)dust_histogram.bin2);
	Serial.print("dust_bin3 :");
	Serial.println((float)dust_histogram.bin3);
	Serial.print("dust_bin4 :");
	Serial.println((float)dust_histogram.bin4);
	Serial.print("dust_bin5 :");
	Serial.println((float)dust_histogram.bin5);
	Serial.print("dust_bin6 :");
	Serial.println((float)dust_histogram.bin6);
	Serial.print("dust_bin7 :");
	Serial.println((float)dust_histogram.bin7);
	Serial.print("dust_bin8 :");
	Serial.println((float)dust_histogram.bin8);
	Serial.print("dust_bin9 :");
	Serial.println((float)dust_histogram.bin9);
	Serial.print("dust_bin10 :");
	Serial.println((float)dust_histogram.bin10);
	Serial.print("dust_bin11 :");
	Serial.println((float)dust_histogram.bin11);
	Serial.print("dust_bin12:");
	Serial.println((float)dust_histogram.bin12);
	Serial.print("dust_bin13 :");
	Serial.println((float)dust_histogram.bin13);
	Serial.print("dust_bin14 :");
	Serial.println((float)dust_histogram.bin14);
	Serial.print("dust_bin15 :");
	Serial.println((float)dust_histogram.bin15);

  }

}


int SensorServer::getAddress(){
return address;

}
