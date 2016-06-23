// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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



