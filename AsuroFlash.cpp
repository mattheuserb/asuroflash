/*
 * Main Class for ASURO Flash Tools
 */

#include "AsuroFlash.h"

#include <glib.h>
#include <time.h>
#include <stdio.h>
#include <glibmm.h>
#include <string.h>

#include "PosixSerial.h"
CPosixSerial Serial;

AsuroFlash::AsuroFlash()
{	
	this->connect_timeout = 10;
	this->flash_timeout = 1;
	this->flash_retry = 5;

	asprintf(&this->config_path, "%s/asuroflash", g_get_user_config_dir());
	read_config();
}

void AsuroFlash::read_config(void)
{	
	FILE *config_file = fopen(config_path, "r");
	char *var, *value;
	
    if(config_file == NULL)
    	return;

	while(fscanf(config_file, "%as = %as\n", &var, &value) > -1) {		
		if(!strcmp(var, "connect_timeout"))
			this->connect_timeout = atoi(value);
		else if(!strcmp(var, "flash_timeout"))
			this->flash_timeout = atoi(value);
		else if(!strcmp(var, "flash_retry"))
			this->flash_retry = atoi(value);
		else if(!strcmp(var, "image_path"))
			this->image_path = strdup(value);
		else if(!strcmp(var, "terminal"))
			this->terminal = strdup(value);
	
		free(var);
		free(value);
		
		var = NULL;
		value = NULL;
	}
	
	fclose(config_file);
}

AsuroFlash::~AsuroFlash()
{
	FILE *config_file = fopen(config_path, "w");
	
    if(config_file == NULL)
    	return;
	
	fprintf(config_file, "connect_timeout = %d\n", this->connect_timeout);
	fprintf(config_file, "flash_timeout = %d\n", this->flash_timeout);
	fprintf(config_file, "flash_retry = %d\n", this->flash_retry);
	fprintf(config_file, "image_path = %s\n", this->image_path);
	fprintf(config_file, "terminal = %s\n", this->terminal);
	
	fclose(config_file);
	// dont free(&this->config_path) its already freed here
}

void AsuroFlash::Init()
{
	if (!Serial.Open(terminal)) {
		error_out("failed");
		throw 1;
	}

	Serial.Timeout(0);
	success_out("OK");
}

void AsuroFlash::Connect()
{
	char buf[11] = {0,0,0,0,0,0,0,0,0,'\0'};
	Serial.ClearBuffer();
	time_t t1,t2;

	time(&t1);
	time(&t2);
	
	normal_out("Connecting to ASURO » ");
	
	while (difftime(t2, t1) < this->connect_timeout) {
		time(&t2);
		
		progress((float)(difftime(t2,t1))/ this->connect_timeout);
		
		//normal_out(".");
		Serial.Write((char*)"Flash", 5);
		Serial.Read(buf, 10);
		TimeWait(100);
		
		if (strstr(buf, "ASURO")) {
			success_out("OK");
			return;
		}
	}
	
	progress(1.0);
	error_out("Timeout");
	throw 1;
}

void AsuroFlash::BuildRAM()
{
	FILE *fp = NULL;
	unsigned int address = 0, type = 0, data = 0, cksum = 0, cksumFile = 0, recordLength = 0, i;
	char tmp[256],line[1024];
	startPage = END_PAGE;
	endPage = START_PAGE;
	
	normal_out("Bulding RAM » ");
	if (image_path[0] != '\0') fp = fopen(image_path, "r");

	if (fp != NULL) {
		rewind(fp);
		while (readLine(line, fp) != EOF) {
			if (line[0] != ':') {
				fclose(fp);
				error_out("wrong file format");
				throw 1;
			}
			
			cksum = 0;
			sscanf(&line[1], "%02X", &recordLength);
			sscanf(&line[3], "%04X", &address);
			sscanf(&line[7], "%02X", &type);
			// get start and end pages
			if ((unsigned int) ((address / (PAGE_SIZE - 2)) +0.5) < startPage)
				startPage = (unsigned int) ((address / (PAGE_SIZE - 3 )) +0.5);
			if ((unsigned int) ((address / (PAGE_SIZE - 2)) +0.5) > endPage)
				endPage = (unsigned int) ((address / (PAGE_SIZE - 3 )) +0.5);
			cksum = recordLength +(unsigned char) ((address & 0xFF00) >> 8)
							+ (unsigned char) (address & 0x00FF) + type;
			
			if (type == 0x00) { // data Record
				int header = HEX_HEADER;
				// Unix \n = 1 char as EOL reading Windows file with \n\r as EOL is 2 char
				if (line[strlen(line)-1] == 0x0a) header += 2;

				if (strlen(line) != recordLength * 2 + header) { 
					error_out("HEX file line length error");
					throw 1;
				}
				
				for ( i = 0; i < recordLength; i++) {
					sscanf(&line[9 + i*2],"%02X",&data);
					cksum += data;
					tmp[i] = data;
				}
				
				cksum = ((~cksum & 0xFF) + 1) & 0xFF;
				sscanf(&line[9 + i*2], "%02X", &cksumFile);
				if (cksum != cksumFile) {
					fclose(fp);
					error_out("HEX file chksum error");
					throw 1;
				}

				if (address + recordLength > (MAX_PAGE * PAGE_SIZE) - BOOT_SIZE) {
					fclose(fp);
					error_out("HEX file too large or address error");
					throw 1;
				}

				memcpy(&RAM[0][0]+address, &tmp[0], recordLength);
			}
			if (type == 0x01) { // End of File Record
				success_out("OK");
				fclose(fp);
				return;
			}
		}
	}

	sprintf(line, "file \"%s\" not found", image_path);
	error_out(line);
	throw 1;
}

void AsuroFlash::SendPage()
{
	unsigned int crc,i,j;
	char sendData[PAGE_SIZE]; 
	char getData[3],tmpText[256];
	time_t t1,t2;
	
	endPage++; // fixed 11.12.2003 
	Serial.Timeout(100);
	for (i = startPage; i <= endPage + 1; i++) {
		sendData[0] = i; // PageNo.
		crc = 0;
		memcpy(&sendData[1],&RAM[i][0],PAGE_SIZE - 3);
		//Build CRC16
		for (j = 0; j < PAGE_SIZE - 2; j++) // -2 CRC16
			crc = CRC16(crc,sendData[j]);
		memcpy(&sendData[j],&crc,2);
		// Last page was send
		if (i == endPage + 1) 
			memset(sendData,0xFF,PAGE_SIZE);
		else {
			sprintf(tmpText, "Sending Page %03d of %03d » ", i, endPage);
			normal_out(tmpText);
		}

		// Try MAX_TRY times before giving up sendig data
		for (j = 0; j < this->flash_retry; j ++) {
			memset(getData,0x00,3);
			Serial.Write(sendData,PAGE_SIZE);
			if ( i == endPage + 1) {
				success_out("All Pages flashed");
				success_out("ASURO ready to start");
				progress(1.0);
				
				return;
			}

			time(&t1);
			time(&t2);
			Serial.ClearBuffer();
			do {
				time(&t2);
	
				Serial.Read(getData, 2);
			} while ((strcmp(getData, "CK") != 0) &&
					 (strcmp(getData, "OK") != 0) &&
					 (strcmp(getData, "ER") != 0) &&
					 difftime(t2, t1) <= this->flash_timeout);

			progress((float)i/(endPage - startPage + 1));
			TimeWait(200);

			if (getData[0] == 'O' && getData[1] == 'K') {
				success_out(" flashed");
				break; // Page successfuly sent
			}
			if (i <= endPage) {
				if (getData[0] == 'C' && getData[1] == 'K')
					warning_out("c"); 
				else if (getData[0] == 'E' && getData[1] == 'R')
					warning_out("v");
				else
					warning_out("t");  
			}
		}
		if (j == this->flash_retry) {
			error_out("\nTimeout");	
			error_out("ASURO not responding (Firmware corrupt, try again)");
			throw 1;
		}
	}
	
	throw 1;
}

void AsuroFlash::Programm()
{
	progress(0.0);
	normal_out("Open Serial Port » ");

	try {
		Init();
		BuildRAM();
		Connect();
		progress(0.0);
		SendPage();
	}
	catch(int a) {

	}
    

	Serial.Close();
}

unsigned int AsuroFlash::CRC16(unsigned int crc, unsigned char data)
{
const unsigned int CRCtbl[256] = {                                 
   0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,   
   0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,   
   0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,   
   0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,   
   0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,   
   0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,   
   0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,   
   0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,   
   0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,   
   0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,   
   0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,   
   0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,   
   0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,   
   0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,   
   0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,   
   0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,   
   0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,   
   0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,   
   0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,   
   0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,   
   0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,   
   0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,   
   0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,   
   0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,   
   0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,   
   0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,   
   0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,   
   0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,   
   0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,   
   0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,   
   0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,   
   0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040 }; 
   return (crc >> 8) ^ CRCtbl[(crc & 0xFF) ^ data];     
}

char AsuroFlash::readLine(char* line, FILE *fp)
{
	char c;
	unsigned int i = 0;
	do {
		c = fgetc(fp);
		if (c == EOF) 
			return EOF;
		line[i++] = c;
	} while (c != '\n');
	line[i] = '\0';
	return true;
}

void AsuroFlash::TimeWait(unsigned int time) // msec
{
	usleep(time * 1000);
}
