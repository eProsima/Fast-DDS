/***
 * Use Case Demonstrator for eProsima Fast RTPS
 * --------------------------------------------
 *
 *  This is an interactive program designed to show the effect of different configuration parameters on the behaviour of eProsima Fast RTPS
 *
 ***/

#include <iostream>
#include <string>

#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/Domain.h>
#include <fastrtps/subscriber/SampleInfo.h>

#include "samplePubSubTypes.h"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

//Enums and configuration structure
enum Reliability_type { Best_Effort, Reliable };
enum Durability_type { Transient_Local, Volatile };
enum HistoryKind_type { Keep_Last, Keep_All };
enum Key_type { No_Key, With_Key};

typedef struct
{
    Reliability_type reliability;
    Durability_type durability;
    HistoryKind_type historykind;
    Key_type keys;
    uint16_t history_size;
    uint8_t depth;
    uint8_t no_keys;
    uint16_t max_samples_per_key;
} example_configuration;

int main()
{

    std::string userchoice;
    bool validinput;
    example_configuration user_configuration = {};


    std::cout << "Welcome to eProsima Fast RTPS Use Case Demonstrator" << std::endl;
    std::cout << "---------------------------------------------------" << std::endl;
    std::cout << "Choose your desired reliability type:" << std::endl;
    std::cout <<
            "1 - Best Effort: Messages are sent with no arrival confirmation. If a sample is lost it cannot be recovered"
              << std::endl;
    std::cout << "2 - Reliable: The Publisher asks for arrival confirmation. Lost samples are re-sent" << std::endl;
    std::cout << "Make your choice (1 or 2): " << std::endl;
    validinput = false;
    while (!validinput)
    {
        std::cin >> userchoice;
        int choice;
        try{
            choice = std::stoi(userchoice);
        }catch (std::invalid_argument&){
            std::cout << "Please input a valid argument" << std::endl;
            continue;
        }
        if (choice == 1)
        {
            user_configuration.reliability = Best_Effort;
            validinput = true;
        }
        else if (choice == 2)
        {
            user_configuration.reliability = Reliable;
            validinput = true;
        }
        else
        {
            std::cout << "Please enter a valid option" << std::endl;
        }
    }
    std::cout << "---------------------------------------------------" << std::endl;
    std::cout << "Choose your desired Durability Type:" << std::endl;
    std::cout << "1 - Transient Local: The Subscriber will receive samples that have been sent before it came online" <<
            std::endl;
    std::cout << "2 - Volatile: The Subscriber receives samples from the moment it comes online, not before:" <<
            std::endl;
    std::cout << "Make your choice (1 or 2): " << std::endl;
    validinput = false;
    while (!validinput)
    {
        std::cin >> userchoice;
        int choice;
        try{
            choice = std::stoi(userchoice);
        }catch (std::invalid_argument&){
            std::cout << "Please input a valid argument" << std::endl;
            continue;
        }
        if (choice == 1)
        {
            user_configuration.durability = Transient_Local;
            validinput = true;
        }
        else if (choice == 2)
        {
            user_configuration.durability = Volatile;
            validinput = true;
        }
        else
        {
            std::cout << "Please enter a valid option" << std::endl;
        }
    }
    std::cout << "---------------------------------------------------" << std::endl;
    std::cout << "Choose your desired History Kind:" << std::endl;
    std::cout <<
            "1 - Keep last: The History stores the last \"k\" received samples. \"k\" is configured as the \"depth\" parameter of the history."
              << std::endl;
    std::cout << "2 - Keep all: The History stores all incoming samples until it is full." << std::endl;
    std::cout << "Make your choice (1 or 2): " << std::endl;
    validinput = false;
    while (!validinput)
    {
        std::cin >> userchoice;
        int choice;
        try{
            choice = std::stoi(userchoice);
        }catch (std::invalid_argument&){
            std::cout << "Please input a valid argument" << std::endl;
            continue;
        }
        if (choice == 1)
        {
            user_configuration.historykind = Keep_Last;
            validinput = true;
        }
        else if (choice == 2)
        {
            user_configuration.historykind = Keep_All;
            validinput = true;
        }
        else
        {
            std::cout << "Please enter a valid option" << std::endl;
        }
    }
    std::cout << "---------------------------------------------------" << std::endl;
    std::cout <<
            "The 'depth' parameter of the History defines how many past samples are stored before starting to overwrite them with newer ones. This only takes effect in 'Keep Last' mode."
              << std::endl;
    std::cout << "Select your desired History depth (enter a number)" << std::endl;
    validinput = false;
    while (!validinput)
    {
        std::cin >> userchoice;
        int choice;
        try{
            choice = std::stoi(userchoice);
        }catch (std::invalid_argument&){
            std::cout << "Please input a valid argument" << std::endl;
            continue;
        }
        user_configuration.depth = (uint8_t)choice;
        validinput = true;
    }
    std::cout << "---------------------------------------------------" << std::endl;
    std::cout <<
            "You can split your History in 'Instances', which act as separate data sinks that end up mapping to 'Keys' on the Publisher side. If you want to use keys, choose a number bigger than one here."
              << std::endl;
    std::cout << "Select your desired maximum number of instances (enter a number)" << std::endl;
    validinput = false;
    while (!validinput)
    {
        std::cin >> userchoice;
        int choice;
        try{
            choice = std::stoi(userchoice);
        }catch (std::invalid_argument&){
            std::cout << "Please input a valid argument" << std::endl;
            continue;
        }
        if (choice > 0)
        {
            user_configuration.depth = (uint8_t)choice;
        }
        else
        {
            user_configuration.depth = 1;
            std::cout << "Defaulting to 1 instance..." << std::endl;
        }
        validinput = true;
    }

    std::cout << "---------------------------------------------------" << std::endl;
    std::cout <<
            "If using more than one instance in the history, you can define the 'depth' on a 'per instance' level. Otherwise, this parameter does not take effect"
              << std::endl;
    std::cout << "Select your desired instance depth (enter a number)" << std::endl;
    validinput = false;
    while (!validinput)
    {
        std::cin >> userchoice;
        int choice;
        try{
            choice = std::stoi(userchoice);
        }catch (std::invalid_argument&){
            std::cout << "Please input a valid argument" << std::endl;
            continue;
        }
        user_configuration.max_samples_per_key = (uint16_t)choice;
        validinput = true;
    }

    std::cout <<
            "By using Keys you can subdivide your topic so your configuration options are applied individually to each subdivision."
              << std::endl;
    std::cout << "Do you want to use keys? (1-yes or 2-no):" << std::endl;
    validinput = false;
    while (!validinput)
    {
        std::cin >> userchoice;
        int choice;
        try{
            choice = std::stoi(userchoice);
        }catch (std::invalid_argument&){
            std::cout << "Please input a valid argument" << std::endl;
            continue;
        }
        if (choice == 1)
        {
            user_configuration.keys = With_Key;
            validinput = true;
        }
        else if (choice == 2)
        {
            user_configuration.keys = No_Key;
            validinput = true;
        }
        else
        {
            std::cout << "Please enter a valid option" << std::endl;
        }
    }
    std::cout << "---------------------------------------------------" << std::endl;
    std::cout <<
            "Lastly, you must define a global maximum size of the History. You must choose a size big enough to hold the amount of samples your previous choices indicate."
              << std::endl;
    std::cout << "Select your desired history size (enter a number)" << std::endl;
    validinput = false;
    while (!validinput)
    {
        std::cin >> userchoice;
        int choice;
        try{
            choice = std::stoi(userchoice);
        }catch (std::invalid_argument&){
            std::cout << "Please input a valid argument" << std::endl;
            continue;
        }
        user_configuration.history_size = (uint16_t)choice;
        validinput = true;
    }

    samplePubSubType sampleType;

    ParticipantAttributes PparamPub;
    PparamPub.rtps.builtin.discovery_config.leaseDuration = c_TimeInfinite;
    PparamPub.rtps.setName("PublisherParticipant");

    Participant* PubParticipant = Domain::createParticipant(PparamPub);
    if (PubParticipant == nullptr)
    {
        std::cout << " Something went wrong while creating the Publisher Participant..." << std::endl;
        return 1;
    }
    Domain::registerType(PubParticipant, (TopicDataType*) &sampleType);

    PublisherAttributes Pparam;
    Pparam.topic.topicDataType = sampleType.getName();
    Pparam.topic.topicName = "samplePubSubTopic";
    Pparam.historyMemoryPolicy = DYNAMIC_RESERVE_MEMORY_MODE;

    if (user_configuration.keys == With_Key)
    {
        Pparam.topic.topicKind = WITH_KEY;
    }
    else
    {
        Pparam.topic.topicKind = NO_KEY;
    }

    if (user_configuration.historykind == Keep_Last)
    {
        Pparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    }
    else
    {
        Pparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
    }

    if (user_configuration.durability == Transient_Local)
    {
        Pparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    }
    else
    {
        Pparam.qos.m_durability.kind = VOLATILE_DURABILITY_QOS;
    }

    if (user_configuration.reliability == Reliable)
    {
        Pparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    }
    else
    {
        Pparam.qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
    }

    Pparam.topic.historyQos.depth = user_configuration.depth;
    Pparam.topic.resourceLimitsQos.max_samples = user_configuration.history_size;
    Pparam.topic.resourceLimitsQos.max_instances = user_configuration.no_keys;
    Pparam.topic.resourceLimitsQos.max_samples_per_instance = user_configuration.max_samples_per_key;


    Publisher* myPub = Domain::createPublisher(PubParticipant, Pparam, nullptr);
    if (myPub == nullptr)
    {
        std::cout << "Somthething went wrong while creating the Publisher..." << std::endl;
        return 1;
    }

    int no_keys = 1;
    sample my_sample;
    if (user_configuration.keys == With_Key)
    {
        no_keys = user_configuration.no_keys;
    }

    std::string c;
    bool condition = true;
    int no;
    while (condition)
    {
        std::cout << "Enter a number to send samples, 'q' to exit" << std::endl;
        std::cin >> c;
        if (c == std::string("q"))
        {
            condition = false;
        }
        else
        {
            try{
                no = std::stoi(c);
            }catch (std::invalid_argument&){
                std::cout << "Please input a valid argument" << std::endl;
                continue;
            }
            for (uint8_t j = 0; j < no; j++)
            {
                for (uint8_t i = 0; i < no_keys; i++)
                {
                    my_sample.index(j + 1);
                    my_sample.key_value(i);
                    myPub->write(&my_sample);
                }
            }
            std::cout << "Sent " << std::to_string(no) << " samples." << std::endl;
        }
    }

    Domain::stopAll();

    return 0;
}
