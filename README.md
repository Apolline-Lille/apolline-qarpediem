# Qarpediem Sensor Node

## General Information

- Qarpediem nodes can be accessed via SSH. Each node must have a hostname which follows this convention :
"qarpediem-[node ID]" with the node ID following this format : Qx (Q1, Q2, Q3, ...) [or Tx (T1, T2, ...) for connected tables]

- Nodes run on Raspbian 8 Jessie (based on Debian 8)
- If you need to use some Raspberry Pins, please use the Wiring Pi library. You can check the sonometer data poller if you need an example of its usage or check this website : http://wiringpi.com/reference/setup/

## Known Issues

- LoRaWAN doesn't work correctly yet. There is a problem with the serial communication between the Raspberry Pi and the Libelium LoRaWAN module. The programs (Qarpediem Data Sender and Sonometer Data Sender) work but the module can't be reached by serial communications at the moment.

- Qarpediem nodes can't be reached via SSH through LILLE1 network because of access limitations from the university and possibly a proxy or firewall which blocks SSH connections (port 22)

- Serial communications latency between Raspberry Pi and Arduino MEGA (around 200 ms or more sometimes) which causes some issues like receiving data from the previous sensor when making requests for each sensor individually. (see InterQarpe Protocol which manages this communication through the Qarpediem Data Poller with a client and through the Arduino with the InterQarpe Server)

## Access LoRaWAN Gateway

The Gateway is accessible, over the small LAN by any computer connected to Linksys-CSAM2 Router (Wifi or Ethernet), via this address : 10.42.0.251

Username and password are written below the gateway.

## Access LoRaWAN Server Admin Interface

The admin interface, located on the data well (which is a Raspberry Pi at the moment),
is accessible via this address by any computer connected to Linksys-CSAM2 Router (Wifi or Ethernet) : https://10.42.0.1:8080

Usernames and passwords are stored in a readme file on a workstation in the Telecom platform

