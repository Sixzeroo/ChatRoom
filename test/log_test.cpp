#include <iostream>
#include <string>

#include "log.h"

int main()
{
	init_logger("debug.log", "info.log", "warn.log", "error.log", "all.log");
	LOG(DEBUG)<<"This is debug test information"<<std::endl;
	LOG(INFO)<<"This is info test information"<<std::endl;
	LOG(WARN)<<"This is warn test information"<<std::endl;
	LOG(ERROR)<<"This is error test information"<<std::endl;
	return 0;
}