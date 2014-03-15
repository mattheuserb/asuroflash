/*
 * Interface ASURO Console Flash
 */

#include "ConAsuroFlash.h"
#include <iostream>

void ConAsuroFlash::progress(float fraction)
{

}

void ConAsuroFlash::normal_out(std::string message)
{
	std::cout << message << std::flush;
}

void ConAsuroFlash::warning_out(std::string message)
{
	std::cout << "\e[1;33m" << message << "\e[0m" << std::flush;
}

void ConAsuroFlash::success_out(std::string message)
{
	std::cout << "\e[1;32m" << message << "\e[0m" << std::endl << std::flush;
}

void ConAsuroFlash::error_out(std::string message)
{
	std::cerr << "\e[1;31m" << message << "\e[0m" << std::endl << std::flush;
}
