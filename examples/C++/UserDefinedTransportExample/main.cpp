/**
 *
 *
 *
 */

#include <cstring>
#include <iostream>

#include "fastrtps/rtps/RTPSDomain.h"

#include "UserDefinedTransportExampleReader.h"
#include "UserDefinedTransportExampleWriter.h"

using namespace eprosima;
using namespace fastrtps;
using namespace rtps;
using namespace std;


int main(int argc, char** argv){

	if(argc > 1)
	{
		if(strcmp(argv[1],"Writer")==0){
			UserDefinedTransportExampleWriter my_writer;
			my_writer.init();
			if(!my_writer.isInitialized()){
				std::cout << "Unable to start a Writer" << std::endl;
				return 1;
			}
			my_writer.sendData();
		}else if(strcmp(argv[1],"Reader")==0){
			UserDefinedTransportExampleReader my_reader;
			my_reader.init();
			if(!my_reader.isInitialized()){
				std::cout << "Unable to start a Reader" << std::endl;
				return 1;
			}
			my_reader.read();
		}
	}
	else
	{
		cout << "Usage: \"UserDefinedTransportExample Writer\" or \"UserDefinedTransportExample Reader\""<<endl;
		return 1;
	}

	RTPSDomain::stopAll();
	cout << "Application exited succesfully"<<endl;

	return 0;
}



