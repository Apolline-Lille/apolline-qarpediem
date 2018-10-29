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

	Serial.println("starting interqarpe interface...");
}

void loop(){
	sensors_server.routine();

}
