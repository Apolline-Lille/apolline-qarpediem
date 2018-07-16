#include <arduPiLoRa.h>
#include <arduPi.h>
#include <SensorsDatabase.h>
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <time.h>
#include <stdint.h>
#define MODE 3  //valeur à modifier en fonction des besoins
#include <signal.h>

using namespace std;

int e;
uint32_t time_for_rasp;
extern SensorsDatabase* database;
static bool lora_initialized = false;


    //STUCT Where will be stored DATA

struct Trame {

    uint8_t header[5];
    uint8_t adresse_du_module_d_envoie[2];
    uint8_t crc_adress;
    float motion;
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
    uint32_t t;
    uint16_t crc_data;
    uint8_t rssi;
}; //__attribute__((packed));

struct time {
    uint8_t header[5];
    uint32_t time;
} __attribute__((packed));

static time_t time_monotonic(){
	struct timespec v;
	clock_gettime(CLOCK_MONOTONIC, &v);
	return v.tv_sec;
}

    //CRC for ADRESSE and DATA
//DATAS
static void CRC_calcCrc8(uint16_t *crcReg, uint16_t poly, uint16_t Data){
  uint16_t i; // pour la boucle for, les datas font 8 bits (char)
  uint16_t xorFlag; // vÃ©rifie si il faut appliquer un xor au CRC a chaque passage dans la boucle ( si le bit de poids fort est Ã  l'Ã©tat haut)
  uint16_t bit;
  uint16_t dcdBitMask = 0x80;
  for(i = 0; i < 8; i++){
    xorFlag = *crcReg & 0x8000; //xorflag =1 si BIT de poid fort etat haut, 0 si non
    *crcReg <<= 1; //dÃ©calage du CRC de 1 Ã  gauche
    bit = ((Data & dcdBitMask) == dcdBitMask);//On vÃ©rifie si le bit sur lequel on se trouve est Ã  l'Ã©tat haut (on dÃ©mare du bit de poid fort)
    *crcReg |= bit;// Si le bit oÃ¹ on se situe est Ã  l'Ã©tat haut , on force le bit de poids faible du CRC Ã  Ãªtre Ã  l'Ã©tait haut ( si il ne l'ai pas dÃ©jÃ )

    if(xorFlag){
      *crcReg = *crcReg ^ poly;// si le bit de poid fort du CRC etait  bien Ã  l'Ã©tat haut, alors on applique le xor avec le polynÃ´me
    }

    dcdBitMask >>= 1;//on continue avec le bit suivant
  }
}

static uint16_t CRC_calc(char Data[],int n){
  uint16_t poly = 0x1021; //poly de division
  uint16_t crcReg= 0xFFFF; //CRC de defaut

  //printf("Value sensor from tab : %d \n", *Data);
  uint8_t *sensordata;
  sensordata = (uint8_t*) Data;

  int i=0;
  //printf("size : %d \n", n);
  for(i=0;i<n;i++){
    //printf("Value sensor from cast: %d \n", *sensordata);
    CRC_calcCrc8(&crcReg,poly,*sensordata); //on modifie le CRC pour chaque octets des data, en reprenant le CRC de l'octets precedent a chaque fois
    sensordata+=1;
    //printf("crcReg_data : %d \n", crcReg);
  }
     //printf("crcReg_data : %d \n", crcReg);
  return crcReg;

}

//ADRESSE
static void CRC_calcCrc8_add(uint8_t *crcReg, uint8_t poly, uint8_t Data){

  uint8_t i; // pour la boucle for, les datas font 8 bits (char)
  uint8_t xorFlag; // vÃ©rifie si il faut appliquer un xor au CRC a chaque passage dans la boucle ( si le bit de poids fort est Ã  l'Ã©tat haut)
  uint8_t bit;
  uint8_t dcdBitMask = 0x80;

  for(i = 0; i < 8; i++){
    xorFlag = *crcReg & 0x80; //xorflag =1 si BIT de poid fort etat haut, 0 si non
    *crcReg <<= 1; //dÃ©calage du CRC de 1 Ã  gauche
    bit = ((Data & dcdBitMask) == dcdBitMask);//On vÃ©rifie si le bit sur lequel on se trouve est Ã  l'Ã©tat haut (on dÃ©mare du bit de poid fort)
    *crcReg |= bit;// Si le bit oÃ¹ on se situe est Ã  l'Ã©tat haut , on force le bit de poids faible du CRC Ã  Ãªtre Ã  l'Ã©tait haut ( si il ne l'ai pas dÃ©jÃ )

    if(xorFlag){
      *crcReg = *crcReg ^ poly;// si le bit de poid fort du CRC etait  bien Ã  l'Ã©tat haut, alors on applique le xor avec le polynÃ´me
       //printf("Value CRCREG 1 : %d \n", *crcReg);
    }

    dcdBitMask >>= 1;//on continue avec le bit suivant
    //printf("Value CRCREG 2 : %d \n", *crcReg);

  }
}

static uint8_t CRC_calc_add(char Data[],int n){
    uint8_t poly = 0x21; //poly de division
    uint8_t crcReg= 0xFF; //CRC de defaut

    //add.ad= 5;
    //add.ade= 15;
    //adreesse[0]=5;
    //adreesse[1]=15;
    //printf("Test adresse data : %d \n", Data);
    uint8_t *adda;
    adda = (uint8_t*) Data;
    //printf("Data :%d \n ", adda);

    int i=0;
    for(i=0;i<n;i++){

    //printf("size : %d \n", n);
    CRC_calcCrc8_add(&crcReg,poly, *adda); //on modifie le CRC pour chaque octets des data, en reprenant le CRC de l'octets precedent a chaque fois
    //printf("adresse : %d \n", adda);
    adda+=1;

    //CRC_calcCrc8_add(&crcReg,poly, adreesse[i]);
  }
    //printf("crcReg_adresse : %d \n", crcReg);

  return crcReg;
}


    //Recover TIME from DATA SINK
static void recover_real_time(uint32_t time_for_struc){

    char header_time[5]= {240, 240, 240, 207, 236};
    e = sx1272.receivePacketTimeoutACK(10000);
    uint32_t * time_from_data_sink;
    uint8_t sum= 0;
    uint8_t check = 1;

    char packet[10];
    int i;
    printf("size data frame : %d \n", sx1272.packet_received.length);
    printf("start loop \n");

        for( i=0 ; i<(sx1272.packet_received.length - 5 ); i++){

        packet[i] = sx1272.packet_received.data[i];
        printf("packet_[%d] : %d \n", i, packet[i]);

            if (i < 5 || sum == 5){
                if(packet[i] == header_time[i]){
                //printf("header [%d] : %s", i, packet[i]);
                sum++;
                }
                if( sum == 5){
                time_from_data_sink = (uint32_t*) &packet[5];
                check = 0;
                sum++;
                }
            }
        }
        if(check == 0){
            time_for_rasp = *time_from_data_sink;
        }else{
            printf("ERROR HEADER"); //add function : send to data sink that there is an error about header
        }


    packet[i] = '\0';
    printf("loop terminated \n");
}

    //Initilisation START and LORA
static void initialisation_frame(){
  uint16_t len = 15;
  uint16_t tem= 1000;
  uint8_t Trame_LoRa_init[16];

  char CRC_calc_2=MODE;
  char adresse[2]={'L',3};
  uint8_t CRC_add = CRC_calc_add(adresse,2);
  int8_t e;

  char trame_init[15] = {240,240,240,240,15,adresse[0],adresse[1],CRC_add,MODE,CRC_calc_add(&CRC_calc_2,1),235,145,123,100,82};

  if (CRC_add==0x00) CRC_add = 0xFF;

  for(int i=0;i<15;i++)Trame_LoRa_init[i]=trame_init[i];

  while(1)
  {
    e = sx1272.sendPacketTimeoutACK(8,Trame_LoRa_init,len,tem);
    if(e == 0)
    {
      break;
    }
  }
  ////////write a logic to establish connection
}

void int_frame(){

    uint8_t trame_init[87] = {170,170,170,170,85,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82};
    uint16_t len = 87;
    uint16_t tem = 1000;

    while(1)
  {
    e = sx1272.sendPacketTimeoutACK(8,trame_init,len,tem);
    if(e == 0)
    {
      break;
    }
  }
}

static void initialisation_SX(){

 // Print a start message
    printf("SX1272 module and Raspberry Pi: send packets with ACK and retries\n");

    // Power ON the module
    e = sx1272.ON();
    printf("Setting power ON: state %d\n", e);

    // Set transmission mode
    e |= sx1272.setMode(4);
    printf("Setting Mode: state %d\n", e);

    // Set header
    e |= sx1272.setHeaderON();
    printf("Setting Header ON: state %d\n", e);

    // Select frequency channel
    e |= sx1272.setChannel(CH_10_868);
    printf("Setting Channel: state %d\n", e);

    // Set CRC
    e |= sx1272.setCRC_ON();
    printf("Setting CRC ON: state %d\n", e);

    // Select output power (Max, High or Low)
    e |= sx1272.setPower('H');
    printf("Setting Power: state %d\n", e);

    // Set the node address
    e |= sx1272.setNodeAddress(3);
    printf("Setting Node address: state %d\n", e);

    // Print a success message
    if (e == 0){
        printf("e : %d", e);
        printf("SX1272 successfully configured\n");
        printf("e : %d", e);
        }
    else{
        printf("SX1272 initialization failed\n");}


}

void print_Byte(uint8_t * tab, float * value_trame){
    printf("adresse tab : %p    ", tab);
    uint8_t* data = (uint8_t*) value_trame;
    for(int i=0; i<4; i++){
        printf("Value[%d] : %x ", i, data[i]);
        tab[i] = data[i];
    }
}

/*static int callback(void *NotUsed, int argc, char **argv, char **azColname) {

    uint8_t tab_frame[87];
    struct Trame trame;
    uint8_t len = 87;
    uint16_t tem = 1000;
    //printf("size : %d", sizeof(trame));
    time_t poll_time = (time_t) atoll(argv[0]);
    

    get_data_for_sensor("motion", poll_time, &trame.motion);
    print_Byte(tab_frame, &trame.motion);
    get_data_for_sensor("sound", poll_time, &trame.sound);
    print_Byte(tab_frame+4,&trame.sound);
    get_data_for_sensor("luminosity", poll_time, &trame.luminosity);
    print_Byte(tab_frame+8,&trame.luminosity);
    get_data_for_sensor("humidity", poll_time, &trame.humidity);
    print_Byte(tab_frame+12,&trame.humidity);
    get_data_for_sensor("pressure", poll_time, &trame.pressure);
    print_Byte(tab_frame+16,&trame.pressure);
    get_data_for_sensor("temperature", poll_time, &trame.temperature);
    print_Byte(tab_frame+20,&trame.temperature);
    get_data_for_sensor("aps1", poll_time, &trame.aps1);
    print_Byte(tab_frame+24,&trame.aps1);
    get_data_for_sensor("aps2", poll_time, &trame.aps2);
    print_Byte(tab_frame+28,&trame.aps2);
    get_data_for_sensor("aps3", poll_time, &trame.aps3);
    print_Byte(tab_frame+32,&trame.aps3);
    get_data_for_sensor("aps4", poll_time, &trame.aps4);
    print_Byte(tab_frame+36,&trame.aps4);
    get_data_for_sensor("aps5", poll_time, &trame.aps5);
    print_Byte(tab_frame+40,&trame.aps5);
    get_data_for_sensor("aps6", poll_time, &trame.aps6);
    print_Byte(tab_frame+44,&trame.aps6);
    get_data_for_sensor("aps7", poll_time, &trame.aps7);
    print_Byte(tab_frame+48,&trame.aps7);
    get_data_for_sensor("aps8", poll_time, &trame.aps8);
    print_Byte(tab_frame+52,&trame.aps8);
    get_data_for_sensor("co2", poll_time, &trame.co2);
    print_Byte(tab_frame+56,&trame.co2);
    get_data_for_sensor("dust::pm1", poll_time, &trame.dust_pm1);
    print_Byte(tab_frame+60,&trame.dust_pm1);
    get_data_for_sensor("dust::pm2.5", poll_time, &trame.dust_pm2_5);
    print_Byte(tab_frame+64,&trame.dust_pm2_5);
    get_data_for_sensor("dust_pm10", poll_time, &trame.dust_pm10);
    print_Byte(tab_frame+68,&trame.dust_pm10);

    trame.t = poll_time;
    printf("Value time : %d \n", trame.t);

    trame.crc_data = CRC_calc((char *) &trame.motion, 18*sizeof(trame.motion));
    printf("Value crc data : %d \n", trame.crc_data);

    trame.rssi = rssi;
    printf("Value RSSI: %d \n ", trame.rssi);

    for(int ty = 0 ; ty <87 ; ty++){
        printf("Packet [%d]: %d \n", ty, tab_frame[ty]);
    }

    
    //e = sx1272.sendPacketTimeoutACKRetries(8, (uint8_t *) &trame, sizeof(trame));

    }

    return 0;
}*/

static void add_data_to_trame(SensorsDatabase::poll_t& poll, string sensor, float* data){
	if(poll.count(sensor) == 1){
		*data = (float) poll[sensor];
	} else {
		*data = (float) nan("");
	}
}

static void send_poll(time_t poll_time){
	struct Trame trame;
	auto poll = database->get_poll(poll_time);
	
	// uint8_t len = 87;
	uint16_t tem = 1000;
	uint8_t rssi = sx1272._RSSI;
	printf("rssi : %d \n", rssi);
	trame.adresse_du_module_d_envoie[0] = 76;
	trame.adresse_du_module_d_envoie[1] = 3;

	trame.header[0] = 0xAA;
	trame.header[1] = 0xAA;
	trame.header[2] = 0xAA;
	trame.header[3] = 0xAA;
	trame.header[4] = 0x55;
	trame.crc_adress = CRC_calc_add((char *) &trame.adresse_du_module_d_envoie, sizeof(trame.adresse_du_module_d_envoie));

	add_data_to_trame(poll, "motion", &trame.motion);
	add_data_to_trame(poll, "sound", &trame.sound);
	add_data_to_trame(poll, "luminosity", &trame.luminosity);
	add_data_to_trame(poll, "humidity", &trame.humidity);
	add_data_to_trame(poll, "pressure", &trame.pressure);
	add_data_to_trame(poll, "temperature", &trame.temperature);
	add_data_to_trame(poll, "aps1", &trame.aps1);
	add_data_to_trame(poll, "aps2", &trame.aps2);
	add_data_to_trame(poll, "aps3", &trame.aps3);
	add_data_to_trame(poll, "aps4", &trame.aps4);
	add_data_to_trame(poll, "aps5", &trame.aps5);
	add_data_to_trame(poll, "aps6", &trame.aps6);
	add_data_to_trame(poll, "aps7", &trame.aps7);
	add_data_to_trame(poll, "aps8", &trame.aps8);
	add_data_to_trame(poll, "co2", &trame.co2);
	add_data_to_trame(poll, "dust_pm1", &trame.dust_pm1);
	add_data_to_trame(poll, "dust_pm2_5", &trame.dust_pm2_5);
	add_data_to_trame(poll, "dust_pm10", &trame.dust_pm10);

	trame.t = (uint32_t) poll_time;
	trame.crc_data = CRC_calc((char *) &trame.motion, 18*sizeof(trame.motion));
	trame.rssi = rssi;
	e = sx1272.sendPacketTimeoutACK(8, (uint8_t*) &trame, sizeof(trame), tem);

	if(e == 0){ // si envoi réussi
		database->poll_sent(poll_time);
	}
	
}

static void send_polls(){
	vector<time_t> polls = database->get_sendable_polls(100000);
	for(const auto& poll_time : polls){
		send_poll(poll_time);
	}
}

static void init_lora(){
    initialisation_SX();
    printf("sending init frame \n");
    initialisation_frame();
    printf("init frame send ! :) \n");
    // recover_real_time(time_for_rasp);
}

void runtime_lora(){
	static time_t last_send = time_monotonic();

	if(!lora_initialized){
		init_lora();
		lora_initialized = true;
	}

	time_t send_interval = (time_t) stoll(database->get_config("interval_lora"));
	time_t now = time_monotonic();
	if(now - last_send >= send_interval){
		cout << "sending poll !" << endl;
		send_polls();
		last_send = now;
	}
}


