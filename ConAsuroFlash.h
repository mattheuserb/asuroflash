#ifndef CONASUROFLASH_H
#define CONASUROFLASH_H

#include "AsuroFlash.h"

class ConAsuroFlash: public AsuroFlash 
{
public:
	void normal_out(std::string message);
	void warning_out(std::string message);
	void success_out(std::string message);
	void error_out(std::string message);
	void progress(float fraction);
};
#endif
