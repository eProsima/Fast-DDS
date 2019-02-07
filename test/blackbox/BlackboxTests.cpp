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

#include "BlackboxTests.hpp"

#include "RTPSAsSocketReader.hpp"
#include "RTPSAsSocketWriter.hpp"
#include "RTPSWithRegistrationReader.hpp"
#include "RTPSWithRegistrationWriter.hpp"
#include "ReqRepAsReliableHelloWorldRequester.hpp"
#include "ReqRepAsReliableHelloWorldReplier.hpp"
#include "TCPReqRepHelloWorldRequester.hpp"
#include "TCPReqRepHelloWorldReplier.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"
#include "PubSubWriterReader.hpp"

#include <fastrtps/rtps/RTPSDomain.h>
#include <fastrtps/rtps/writer/WriterListener.h>
#include <fastrtps/rtps/flowcontrol/ThroughputControllerDescriptor.h>
#include <fastrtps/transport/UDPv4Transport.h>
#include <fastrtps/transport/test_UDPv4Transport.h>
#include <fastrtps/rtps/resources/AsyncWriterThread.h>
#include <fastrtps/rtps/common/Locator.h>
#include <fastrtps/xmlparser/XMLParser.h>

#include <thread>
#include <memory>
#include <cstdlib>
#include <string>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

#include "BlackboxTestsUtils.hpp"

class BlackboxEnvironment : public ::testing::Environment
{
    public:

        void SetUp()
        {
            global_port = get_port();
            //Log::SetVerbosity(Log::Info);
            //Log::SetCategoryFilter(std::regex("(SECURITY)"));
        }

        void TearDown()
        {
            //Log::Reset();
            Log::KillThread();
            eprosima::fastrtps::rtps::RTPSDomain::stopAll();
        }
};

#include "BlackboxTestsRTPS.hpp"
#include "BlackboxTestsPubSub.hpp"
#include "BlackboxTestsDiscovery.hpp"
#include "BlackboxTestsPersistence.hpp"

#if HAVE_SECURITY
#include "BlackboxTestsSecurity.hpp"
#endif

#include "BlackboxTestsTransportUDP.hpp"
#include "BlackboxTestsTransportTCP.hpp"

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    testing::AddGlobalTestEnvironment(new BlackboxEnvironment);

#if HAVE_SECURITY
    certs_path = std::getenv("CERTS_PATH");

    if(certs_path == nullptr)
    {
        std::cout << "Cannot get enviroment variable CERTS_PATH" << std::endl;
        exit(-1);
    }
#endif

    return RUN_ALL_TESTS();
}
