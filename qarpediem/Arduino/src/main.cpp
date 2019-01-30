#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

#include <AD7193.h>
#include <AlphaSense.h>
#include <ChainableLED.h>
#include <DHT.h>
#include <Digital_Light_TSL2561.h>
#include <DuplexBase.h>
#include <Multiplexer.h>
#include <SensorAverage.h>
#include <Qarpediem.h>

using namespace Qarpediem;

Qarpediem::SensorServer sensors_server(Qarpediem::D_SUB_SERIAL);
void setup(){
	Serial.begin(9600);
	Serial.println();
	Serial.println("----------------------------");
	Serial.println("| QARPEDIEM - Sensor Board |");
	Serial.println("----------------------------");
	Serial.flush();

	sensors_server.init();

/*	Serial.println("starting interqarpe interface...");
	Serial.println("configuring OPCN2 (dust sensor)...");
	delay(1000);
	dust_sensor.begin(Qarpediem::SS_DUST);
	dust_sensor.on();
	delay(8000);
	if(!dust_sensor.ping()){
		Serial.println("> can't contact the dust sensor");
	}
	Serial.println("> dust sensor configure_adc");

	dust_histogram = dust_sensor.read_histogram();


	Serial.println("Configuring pin");
	pinMode(Qarpediem::ANALOG_SON_ENVOLOPE, INPUT);
	pinMode(Qarpediem::ANALOG_PRESSION, INPUT);
	pinMode(Qarpediem::DIG_MVT, INPUT);

	Serial.flush();*/
}

void loop(){
	sensors_server.routine();
	/*bool var=dust_sensor.ping();
	Serial.println(var?"True":"False");
	delay(1000);
	dust_histogram=dust_sensor.read_histogram();
	Serial.print("PM10: ");
	Serial.println(dust_histogram.pm10);
	delay(1000);*/


}
