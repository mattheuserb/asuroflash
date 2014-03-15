/*
 * Serial Communictaion in Posix standard
 */
 
#include <stdio.h>
#include <iostream>
#include <stdlib.h>

#include "PosixSerial.h"
bool CPosixSerial::Open(const char* ttyS)
{
	char text[256];
	
	this->ttyS = ttyS;
	
	portHandle = open(ttyS, O_RDWR | O_NOCTTY);
	if (portHandle == -1) {
		sprintf(text, "Could not open %s\nAlready in use?\n", ttyS);
		MyMessageBox(text);
		return false;
	}
	// configure port settings
	tcgetattr(portHandle, &CommConfig);

	// 2400 Baud / Data Size 8-Bit / 1 Stop Bit / No Parity / No Flow Control / Zero TimeOut
	CommConfig.c_cflag = CREAD|CLOCAL|B2400|CS8;
	CommConfig.c_lflag = 0;
	CommConfig.c_oflag = 0;
	CommConfig.c_iflag = 0;	CommConfig.c_cc[VMIN] = 0;
	CommConfig.c_cc[VTIME]= 0;

	// Set DTR & RTS
	ioctl(portHandle, TIOCMSET, TIOCM_DTR | TIOCM_RTS);

	if (tcsetattr(portHandle, TCSAFLUSH, &CommConfig)) {
		sprintf(text, "Could not write port settings on %s\n", ttyS);
		MyMessageBox(text);
		return false;
	}
 	
	return true;
}

void CPosixSerial::Close(void)
{
	close(portHandle);
}

void CPosixSerial::ClearBuffer(void)
{
}

int CPosixSerial::Read(char* data, unsigned int length)
{
	int ret;
	char text[256];
	ret = read(portHandle, data, length);

	if (ret == -1) {
		sprintf(text, "Could not read from %s", ttyS);
		MyMessageBox(text); 
	} 

	return ret;	
}

int CPosixSerial::Write(char* data, unsigned int length)
{
	int ret;
	char text[256];
	ret = write(portHandle, data, length);

	if (ret == -1) {
		sprintf(text, "Could not write to %s", ttyS);
		MyMessageBox(text);
	}

	return ret; 
}

void CPosixSerial::Timeout(unsigned int timeout) //msec
{
  CommConfig.c_cc[VTIME]=timeout/100;
  tcsetattr(portHandle, TCSAFLUSH, &CommConfig);
}

void CPosixSerial::MyMessageBox(char* text)
{
	std::cout << text << std::endl;
}
	
