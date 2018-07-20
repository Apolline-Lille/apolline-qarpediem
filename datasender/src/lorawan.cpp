/**
* lorawan.cpp
* Implementation of the LoRaWAN protocol for the Qarpediem Data Sender
* using the ArduPiLoRaWAN API.
*
* Contributor: Sébastien Maes
*/
#include <iostream>
#include <string>
#include <fstream>

#include <time.h>
#include <vector>
#include <cstring>
#include <stdint.h>
#include <arduPi.h>
#include <arduPiLoRaWAN.h>
#include <sqlite3.h>
#include <SensorsDatabase.h>

using namespace std;

//--------------------------------------------------------------------
int status;
extern SensorsDatabase *database;
static bool lorawan_initialized = false;
const int NODE_ADDRESS_SIZE = 2;
//--------------------------------------------------------------------

// Device parameters for Back-End registration
//-------------------------------------------------------------------
//char NWK_SESSION_KEY[] = "17EBA04EEA7623139F259D52AE348A2F"; // ABP-only
//char APP_SESSION_KEY[] = "D7523CDA0B3F9CB7CFF9BA9474F479B9"; // ABP-only
char APP_KEY[] = "D7523CDA0B3F9CB7CFF9BA9474F479F9"; // 128-bit
char APP_EUI[] = "CFF9BA9474F479B9";                 // 64-bit
uint8_t PORT = 3;                                    // Define port to use in Back-End: from 1 to 223
uint8_t POWER = 1;                                   // 0 -> 20 dBm (if supported), 1 -> 14 dBm, 2 -> 11 dBm, 3 -> 8 dBm, 4 -> 5 dBm, 5 -> 2 dBm
//--------------------------------------------------------------------

// STRUCTURE which contains data
struct Payload
{
    char node_address[NODE_ADDRESS_SIZE]; // 2 bytes
    float motion;                         // float -> 4 bytes
    float sound;
    float luminosity;
    float humidity;
    float pressure;
    float temperature;
    float aps1;
    float aps2;
    float aps3;
    float aps4;
    float aps5;
    float aps6;
    float aps7;
    float aps8;
    float co2;
    float dust_pm1;
    float dust_pm2_5;
    float dust_pm10;
    uint32_t poll_time; // 4 bytes
};

void write_device_eui(){
	ofstream file("/home/qarpediem/lorawan-device-eui.dat", ios::out);
	string deviceEUI = LoRaWAN._devEUI;
	file.write(deviceEUI.c_str(), deviceEUI.length());
	file.close();
}


static time_t time_monotonic()
{
    struct timespec v;
    clock_gettime(CLOCK_MONOTONIC, &v);
    return v.tv_sec;
}

static void add_data_to_payload(SensorsDatabase::poll_t &poll, string sensor, float *data)
{
    if (poll.count(sensor) == 1)
    {
        *data = (float)poll[sensor];
    }
    else
    {
        *data = (float)nan(""); // Not a number
    }
}

static void send_poll(time_t poll_time)
{
    struct Payload dataPayload;
    size_t payloadSize = sizeof(struct Payload);
    auto poll = database->get_poll(poll_time);

    strcpy(dataPayload.node_address, database->get_config("node_address").c_str());

    add_data_to_payload(poll, "motion", &dataPayload.motion);
    add_data_to_payload(poll, "sound", &dataPayload.sound);
    add_data_to_payload(poll, "luminosity", &dataPayload.luminosity);
    add_data_to_payload(poll, "humidity", &dataPayload.humidity);
    add_data_to_payload(poll, "pressure", &dataPayload.pressure);
    add_data_to_payload(poll, "temperature", &dataPayload.temperature);
    add_data_to_payload(poll, "aps1", &dataPayload.aps1);
    add_data_to_payload(poll, "aps2", &dataPayload.aps2);
    add_data_to_payload(poll, "aps3", &dataPayload.aps3);
    add_data_to_payload(poll, "aps4", &dataPayload.aps4);
    add_data_to_payload(poll, "aps5", &dataPayload.aps5);
    add_data_to_payload(poll, "aps6", &dataPayload.aps6);
    add_data_to_payload(poll, "aps7", &dataPayload.aps7);
    add_data_to_payload(poll, "aps8", &dataPayload.aps8);
    add_data_to_payload(poll, "co2", &dataPayload.co2);
    add_data_to_payload(poll, "dust_pm1", &dataPayload.dust_pm1);
    add_data_to_payload(poll, "dust_pm2_5", &dataPayload.dust_pm2_5);
    add_data_to_payload(poll, "dust_pm10", &dataPayload.dust_pm10);

    dataPayload.poll_time = (uint32_t)poll_time;

    status = LoRaWAN.sendConfirmed(PORT, (uint8_t *)&dataPayload, (uint16_t)sizeof(struct Payload));

    cout << "Send Payload: state " << status << endl;
    if (status == 0)
    { // si envoi réussi
        database->poll_sent(poll_time);
        cout << "Payload sent successfully" << endl;
    }
}

static void send_polls()
{
    vector<time_t> polls = database->get_sendable_polls(100000);
    for (const auto &poll_time : polls)
    {
        send_poll(poll_time);
    }
    send_poll(time(NULL));
}

static int initLoraWanModule()
{
    // Print a start message
    cout << "LoRaWAN module and Raspberry Pi: Send packets with ACK and retries" << endl;

    // Power ON the module
    status = LoRaWAN.ON(SOCKET0);
    cout << "Setting power ON: state " << status << endl;

    // Get the preprogrammed unique identifier
    status |= LoRaWAN.getEUI();
    cout << "Get preprogrammed EUI: state " << status << endl;

    // Set/Get Device using preprogrammed unique identifier
    status |= LoRaWAN.setDeviceEUI();
    status |= LoRaWAN.getDeviceEUI();
    cout << "Set & Get Device EUI: state " << status << endl;

    // Set Network Session Key (ABP-only)
    //status |= LoRaWAN.setNwkSessionKey(NWK_SESSION_KEY);

    // Set App Session Key (ABP-only)
    //LoRaWAN.setAppSessionKey(APP_SESSION_KEY);

    // Set retransmissions for uplink confirmed packet
    status |= LoRaWAN.setRetries(7);
    status |= LoRaWAN.getRetries();
    cout << "Set Retries: state " << status << endl;

    // Set Application Key
    status |= LoRaWAN.setAppKey(APP_KEY);
    cout << "Set App Key: state " << status << endl;

    // Set/Get Application EUI
    status |= LoRaWAN.setAppEUI(APP_EUI);
    status |= LoRaWAN.getAppEUI();
    cout << "Set/Get Application EUI: state " << status << endl;

    // Set/Get Adaptive data rate
    status |= LoRaWAN.setADR("on");
    status |= LoRaWAN.getADR();
    cout << "Enable Adaptive Data Rate: state " << status << endl;

    // Join a network (Over The Air Activation)
    status |= LoRaWAN.joinOTAA();
    cout << "Join a network (OTAA): state " << status << endl;
    if (status == 0)
    {
        cout << "Module successfully joined a network" << endl;
    }
    else
    {
        cout << "Module failed to join a network" << endl;
    }

    // SAVE Configuration
    status |= LoRaWAN.saveConfig();
    if (status == 0)
    {
        cout << "Configuration successfully SAVED" << endl;
    }
    else
    {
        cout << "Configuration not saved" << endl;
    }

    // Useful information about config
    cout << "=== LoRaWAN Module Config Recap ===" << endl;
    cout << "EUI: " << LoRaWAN._eui << endl;
    cout << "Device EUI: " << LoRaWAN._devEUI << endl;
    cout << "App Key: " << LoRaWAN._appKey << endl;
    cout << "App EUI: " << LoRaWAN._appEUI << endl;

    if (status == 0)
    {
        cout << "LoRaWAN module SUCCESSFULLY configured" << endl;
    }
    else
    {
        cout << "LoRaWAN module initialization FAILED" << endl;
    }

    return status;
}

void runtime_lorawan()
{
    int init(0);
    static time_t last_send = time_monotonic();
    long long interval = stoll(database->get_config("interval_lora"));
    if (!lorawan_initialized)
    {
        do
        {
            cout << "LoRaWAN module initialization..." << endl;
            init = initLoraWanModule();
        } while (init != 0);
        write_device_eui();
        database->set_config("interval_lora", interval);
        database->set_config("interval_polling", interval);
        lorawan_initialized = true;
    }

    time_t send_interval = (time_t)interval;
    time_t now = time_monotonic();
    if (now - last_send >= send_interval)
    {
        cout << "Sending poll..." << endl;
        send_polls();
        last_send = now;
    }
}
