#ifndef ASUROFLASH_H
#define ASUROFLASH_H

#include <stdio.h>
#include <iostream>

#define START_PAGE 0x0000
#define END_PAGE   0x2000

#define MAX_PAGE  128    // AtmelMega8 8kByte Flash
#define PAGE_SIZE 64 + 3 // Bytes +1PageNo +2 CRC16
#define BOOT_SIZE 1024   // Bytes

#define HEX_HEADER 1+2+4+2+2 // : + recordLength + address + type + chksum

class AsuroFlash
{
public:
	void Programm();
	void read_config();
	AsuroFlash();
	virtual ~AsuroFlash();

	virtual void normal_out(std::string) {};
	virtual void warning_out(std::string) {};
	virtual void success_out(std::string) {};
	virtual void error_out(std::string) {};
	virtual void progress(float) {};

private:
	void SendPage();
	void BuildRAM();
	void Connect();
	void Init();
	void TimeWait(unsigned int time); //msec
	unsigned int CRC16 (unsigned int crc, unsigned char data);
	char readLine(char* line, FILE* fp);

public:
	char *terminal;
	char *image_path;

private:
	char *config_path;
	unsigned int connect_timeout;	// sec
	unsigned int flash_timeout;  // sec
	unsigned int flash_retry;

	unsigned int startPage, endPage;
	unsigned char RAM[MAX_PAGE][PAGE_SIZE - 3]; //-1PageNo -2CRC16 
};
#endif
