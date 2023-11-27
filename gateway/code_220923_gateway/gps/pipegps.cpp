/*
	GPS L80-R 
	I2C SC16IS750
	i2cdetect -y 8 => 0x48
	file wiringpii2c.c edit i2cdetect -> 8
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wiringPi.h>
#include "sc16is750.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

#define I2C_ADDRESS 0x48
#define BAUDRATE 9600

const char *PMTK = "$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n";
const char *INTERVAL = "$PMTK220,7000*2F\r\n";
const char *PATH = "/tmp/myfifo";
typedef struct{
	double gps_lat;
	double gps_lng;
	float gps_speed;
	float gps_or;
}GPS;
GPS pos = {0,0,0,0};


typedef struct{
	uint16_t deg;
	uint32_t billion;
}DEGREES;
DEGREES degreeslat,degreeslng;

char buffer[128];
uint16_t index_read = 0;
char *datapipe;
uint8_t cout=0;


void pipe_write(char* buf,const char* path);

void nmea_gprmc_decoder(GPS *gps_data,char* data);
double gps_convert_lat_to_double(char* lat,char *ns);
double gps_convert_lng_to_double(char* lng,char *we);

void sc16is750_init(void);
void sc16is750_read(void);
void sc16is750_write(const char *cmd);
void parse_degrees(const char *term, DEGREES &deg);

SC16IS750_t i2cuart;
int main(int argc, char **argv){
	mkfifo(PATH,0666);
	sc16is750_init();
	sc16is750_write(INTERVAL);
	sleep(0.1);
	sc16is750_write(PMTK);
	sleep(0.1);


	while(1) {
		sc16is750_read();	

	} 
}


void pipe_write(char* buf,const char* path){
	
	int pipe;
	pipe = open(path,O_WRONLY);
	write(pipe,buf,strlen(buf)+1);
	close(pipe);
	
	//test printf data gps when close pipe
	//printf("pipe write:%s\r\n",buf);
}

/*
	init sc16is750 với i2c address 0x48
	baud 9600
*/
void sc16is750_init(void){
	printf("----------init sc16is750-------------");
	SC16IS750_init(&i2cuart, SC16IS750_PROTOCOL_I2C, I2C_ADDRESS, SC16IS750_SINGLE_CHANNEL);
	if(wiringPiSetup() == -1) {
		printf("wiringPiSetup Fail\n");
		return;
	}
	SC16IS750_begin(&i2cuart, BAUDRATE, SC16IS750_CHANNEL_NONE, 14745600UL); //baudrate&frequency setting
	if (SC16IS750_ping(&i2cuart)!=1) {
		printf("device not found\n");
		return;
	} else {
		printf("device found\n");
	}
}

/*
	gửi cmd to gps reci GPRMC
*/
void sc16is750_write(const char *cmd){
	uint8_t len = strlen(cmd);
	uint8_t index = 0;
	for(index = 0;index < len; index++){
		SC16IS750_write(&i2cuart,SC16IS750_CHANNEL,cmd[index]);
	}
}
void sc16is750_read(void){
	while(SC16IS750_available(&i2cuart,SC16IS750_CHANNEL) == 0);
	char c =SC16IS750_read(&i2cuart,SC16IS750_CHANNEL);
	if(c == 0x0a){
		cout ++;
		printf("Data:%s\n",buffer);
		nmea_gprmc_decoder(&pos,buffer);
		if(cout == 8){
		datapipe = (char*)malloc(128*sizeof(char));
		sprintf(datapipe,"\"gps\":{\"lat\":\"%f\",\"lng\":\"%f\",\"speed\":\"%.2f\"}",pos.gps_lat,pos.gps_lng,pos.gps_speed);
		pipe_write(datapipe,PATH);
		cout=0;
		free(datapipe);
		}
		//printf("%s\n",datapipe);
		//printf("------------------\n");
		//printf("%f\n",pos.gps_lat);
		//printf("%f\n",pos.gps_lng);
		//printf("%f\n",pos.gps_speed);
		//printf("%f\n",pos.gps_or);
		index_read = 0;
		buffer[0] = 0;
		memset(buffer,'\0',sizeof(buffer));
	}
	else{
		if(index_read < sizeof(buffer)-1){
			buffer[index_read++] = c;
			buffer[index_read] = 0;
		}
	}
}



void nmea_gprmc_decoder(GPS *gps_data,char *data){
	char format[20][15];
	char *token = strtok(data,",");
	uint8_t i=0;
	try{
	while(token != NULL){
		strcpy(format[i],token);
		token = strtok(NULL,",");
		i++;
	}
	if(strcmp(format[0],"$GPRMC") !=0){
		return;
	}
	if(strcmp(format[2],"V") == 0){
		gps_data-> gps_lat = 0;
		gps_data-> gps_lng = 0;
		gps_data-> gps_speed = 0;
		gps_data-> gps_or = 0;
	}
	else if(strcmp(format[2],"A") == 0){
		gps_data-> gps_lat = gps_convert_lat_to_double(format[3],format[4]);
		gps_data-> gps_lng = gps_convert_lng_to_double(format[5],format[6]);
		gps_data-> gps_speed = atof(format[7])*1.852;//knot to km/h
		gps_data-> gps_or = atof(format[8]);
		
	}
	}
	catch(...){
		printf("contiunus\n");
	}
}

double gps_convert_lat_to_double(char *lat, char *ns){
	double ret;
	parse_degrees(lat,degreeslat);
	ret = degreeslat.deg + degreeslat.billion / 1000000000.0;
	if(strcmp(ns,"S") == 0){
		return -ret;
	}
	return ret;
}
double gps_convert_lng_to_double(char *lng,char *we){
	double ret;
	parse_degrees(lng,degreeslng);
	ret = degreeslng.deg + degreeslng.billion /1000000000.0;
	if(strcmp(we,"W") == 0){
		return -ret;
	}
	return ret;
}
void parse_degrees(const char *term, DEGREES &deg)
{
  uint32_t leftOfDecimal = (uint32_t)atol(term);
  uint16_t minutes = (uint16_t)(leftOfDecimal % 100);
  uint32_t multiplier = 10000000UL;
  uint32_t tenMillionthsOfMinutes = minutes * multiplier;
 
  deg.deg = (int16_t)(leftOfDecimal / 100);

  while (isdigit(*term))
    ++term;

  if (*term == '.')
    while (isdigit(*++term))
    {
      multiplier /= 10;
      tenMillionthsOfMinutes += (*term - '0') * multiplier;
    }

  deg.billion = (5 * tenMillionthsOfMinutes + 1) / 3;
  // printf("------------------\n");
  // printf("%d\n",deg.deg);
  // printf("%d\n",deg.billion);
  // printf("------------------\n");
}
