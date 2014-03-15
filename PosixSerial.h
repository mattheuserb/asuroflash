#ifndef POSIXSERIAL_H
#define POSIXSERIAL_H

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/select.h>

class CPosixSerial
{
public:
	bool Open(const char* ttyS);
	void Close(void);
	void ClearBuffer(void);
	int Read(char* data, unsigned int length);
	int Write(char* data, unsigned int length);
	void Timeout(unsigned int timeout); //msec

protected :
  int portHandle;
  const char* ttyS; 
  struct termios CommConfig;
	
private :
	void MyMessageBox(char* Text);
};
#endif

