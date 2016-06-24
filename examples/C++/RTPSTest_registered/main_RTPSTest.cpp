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

#include <stdio.h>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <bitset>
#include <cstdint>
#include <sstream>

#include "fastrtps/log/Log.h"
#include "fastrtps/rtps/RTPSDomain.h"

#include "TestWriterRegistered.h"
#include "TestReaderRegistered.h"

using namespace eprosima;
using namespace fastrtps;
using namespace rtps;
using namespace std;


int main(int argc, char** argv){
	cout << "Starting RTPS example" << endl;
	int type;
	if(argc > 1)
	{
		if(strcmp(argv[1],"writer")==0)
			type = 1;
		else if(strcmp(argv[1],"reader")==0)
			type = 2;
		else
		{
			cout << "NEEDS writer OR reader as first argument"<<endl;
			return 0;
		}
	}
	else
	{
		cout << "NEEDS writer OR reader ARGUMENT"<<endl;
		cout << "RTPSTest writer"<<endl;
		cout << "RTPSTest reader" <<endl;
		return 0;
	}
	switch (type)
	{
	case 1:
	{
		TestWriterRegistered TW;
		if(TW.init() && TW.reg())
			TW.run(10);
		break;
	}
	case 2:
	{
		TestReaderRegistered TR;
		if(TR.init() && TR.reg())
			TR.run();
		break;
	}
	}

	RTPSDomain::stopAll();
	cout << "EVERYTHING STOPPED FINE"<<endl;

	return 0;
}



