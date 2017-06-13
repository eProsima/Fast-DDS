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

#include <chrono>

#include "deadlineQoS.h"
#include <string>

using namespace asio;
using namespace eprosima;
using namespace eprosima::fastrtps;

void deadlineQoS::callback()
{
	mapmtx.lock();
	std::cout << "Map holds " << deadlineQoSmap.size() << " different keys" << std::endl;
	for(auto it = deadlineQoSmap.begin(); it != deadlineQoSmap.end(); it++){

		if(it->second == false){
			std::cout << "Deadline QoS on key index ";
			std::cout << static_cast<int>(it->first.value[1]);
			std::cout << " missed." << std::endl;
		}
		//After checking the value, reset flag to zero
		it->second = false;
	}
	mapmtx.unlock();
	wait();
}

void deadlineQoS::wait()
{
	 t.expires_from_now(std::chrono::seconds(1)); //repeat rate here
     t.async_wait(std::bind(&deadlineQoS::callback, this));
}

void deadlineQoS::setFlag(mapable_key target)
{
	mapmtx.lock();
	if(deadlineQoSmap.find(target)!=deadlineQoSmap.end()){
		//Exists
		deadlineQoSmap.at(target)=true;
	}else{
		//Does not exist
		deadlineQoSmap.insert(std::pair<mapable_key,bool>(target,true));
	}
	//deadlineQoSmap.[target]=true;
	mapmtx.unlock();
	//if(index<32){
	//	deadlineQoSlist[index].mtx.lock();
	//	deadlineQoSlist[index].flag = true;
	//	deadlineQoSlist[index].mtx.unlock();
	//}
}

void deadlineQoS::init(){
	//Not really needed now
	std::cout << "Deadline QoS Service started" << std::endl;
	//for(int i=0;i<32;i++){
	//	deadlineQoSlist[i].mtx.lock();
	//	deadlineQoSlist[i].flag = false;
	//	deadlineQoSlist[i].mtx.unlock();
	//}
}

void deadlineQoS::runner(){
		wait();

		io.run();
}

void deadlineQoS::run(){
	dlqos = new std::thread(std::bind(&deadlineQoS::runner,this));
}

void deadlineQoS::stop(){
	t.cancel();
	io.stop();
	dlqos->join();
}
