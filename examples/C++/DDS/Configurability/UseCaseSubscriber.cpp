/***
 * Use Case Demonstrator for eProsima Fast RTPS
 * --------------------------------------------
 *
 *  This is an interactive program designed to show the effect of different configuration parameters on the behaviour of eProsima Fast RTPS
 *
 ***/

#include <iostream>
#include <string>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>

#include "samplePubSubTypes.h"

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps::rtps;

//Enums and configuration structure
enum Reliability_type { Best_Effort, Reliable };
enum Durability_type { Transient_Local, Volatile };
enum HistoryKind_type { Keep_Last, Keep_All };

typedef struct
{
    Reliability_type reliability;
    Durability_type durability;
    HistoryKind_type historykind;
    uint16_t history_size = 1;
    uint8_t depth = 1;
    uint8_t no_keys = 1;
    uint16_t max_samples_per_key = 1;
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
    if (user_configuration.historykind == Keep_Last)
    {
        std::cout << "---------------------------------------------------" << std::endl;
        std::cout <<
            "The 'depth' parameter of the History defines how many samples are stored before starting to overwrite them with newer ones."
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
            user_configuration.no_keys = (uint8_t)choice;
        }
        else
        {
            user_configuration.no_keys = 1;
            std::cout << "Defaulting to 1 instance..." << std::endl;
        }
        validinput = true;
    }

    if (user_configuration.no_keys > 1)
    {
        std::cout << "---------------------------------------------------" << std::endl;
        std::cout <<
            "If using more than one instance in the history, you can define the 'depth' on a 'per instance' level (Max Samples per Instance). Otherwise, this parameter does not take effect"
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
    }
    std::cout << "---------------------------------------------------" << std::endl;
    std::cout <<
        "Lastly, you must define a global maximum size of the History (Max Samples). You must choose a size big enough to hold the amount of samples your previous choices indicate."
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

    TypeSupport sampleType(new samplePubSubType());

    DomainParticipantQos pqos;

    pqos.wire_protocol().builtin.discovery_config.leaseDuration = eprosima::fastrtps::c_TimeInfinite;
    pqos.name("SubscriberParticipant");

    DomainParticipant* SubParticipant = DomainParticipantFactory::get_instance()->create_participant(0, pqos);
    if (SubParticipant == nullptr)
    {
        std::cout << " Something went wrong while creating the Subscriber Participant..." << std::endl;
        return 1;
    }
    //Register the type
    sampleType.register_type(SubParticipant);

    //Create Subscriber
    Subscriber* EarlySubscriber = SubParticipant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    if (EarlySubscriber == nullptr)
    {
        std::cout << "Something went wrong while creating the Subscriber..." << std::endl;
        return 1;
    }

    //Create Topic
    Topic* SubTopic = SubParticipant->create_topic("samplePubSubTopic", sampleType.get_type_name(), TOPIC_QOS_DEFAULT);

    if (SubTopic == nullptr)
    {
        std::cout << "Something went wrong while creating the Subscriber Topic..." << std::endl;
        return 1;
    }

    //Create DataReader
    DataReaderQos rqos;
    rqos.endpoint().history_memory_policy = DYNAMIC_RESERVE_MEMORY_MODE;

    if (user_configuration.historykind == Keep_Last)
    {
        rqos.history().kind = KEEP_LAST_HISTORY_QOS;
    }
    else
    {
        rqos.history().kind = KEEP_ALL_HISTORY_QOS;
    }

    if (user_configuration.durability == Transient_Local)
    {
        rqos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    }
    else
    {
        rqos.durability().kind = VOLATILE_DURABILITY_QOS;
    }

    if (user_configuration.reliability == Reliable)
    {
        rqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    }
    else
    {
        rqos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;
    }

    rqos.history().depth = user_configuration.depth;
    rqos.resource_limits().max_samples = user_configuration.history_size;
    rqos.resource_limits().max_instances = user_configuration.no_keys;
    rqos.resource_limits().max_samples_per_instance = user_configuration.no_keys > 1 ?
        user_configuration.max_samples_per_key : user_configuration.history_size;

    DataReader* EarlyReader = EarlySubscriber->create_datareader(SubTopic, rqos);
    if (EarlyReader == nullptr)
    {
        std::cout << "Something went wrong while creating the Subscriber DataReader..." << std::endl;
        return 1;
    }

    std::cout << "Subscriber online" << std::endl;
    std::string c;
    bool condition = true;
    sample my_sample;
    SampleInfo sample_info;
    while (condition)
    {
        std::cout << "Press 'r' to read Messages from the History or 'q' to quit" << std::endl;
        std::cin >> c;
        if ( c == std::string("q") )
        {
            condition = false;
        }
        else if ( c == std::string("r") )
        {
            while (EarlyReader->read_next_sample(&my_sample, &sample_info) == ReturnCode_t::RETCODE_OK)
            {
                std::cout << "Sample Received! Index:" << std::to_string(my_sample.index()) << " Key:" <<
                    std::to_string(my_sample.key_value()) << std::endl;
            }
        }
    }

    EarlySubscriber->delete_datareader(EarlyReader);
    SubParticipant->delete_subscriber(EarlySubscriber);
    SubParticipant->delete_topic(SubTopic);
    DomainParticipantFactory::get_instance()->delete_participant(SubParticipant);

    return 0;
}
