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
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/topic/Topic.hpp>

#include "samplePubSubTypes.h"

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps::rtps;

//Enums and configuration structuredepth
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

    //Create Participant
    DomainParticipantQos pqos;
    pqos.wire_protocol().builtin.discovery_config.leaseDuration = eprosima::fastrtps::c_TimeInfinite;
    pqos.name("PublisherParticipant");

    DomainParticipant* PubParticipant = DomainParticipantFactory::get_instance()->create_participant(0, pqos);
    if (PubParticipant == nullptr)
    {
        std::cout << " Something went wrong while creating the Publisher Participant..." << std::endl;
        return 1;
    }
    //Register the type
    sampleType.register_type(PubParticipant);

    //Create the Publisher
    Publisher* myPub = PubParticipant->create_publisher(PUBLISHER_QOS_DEFAULT);
    if (myPub == nullptr)
    {
        std::cout << "Something went wrong while creating the Publisher..." << std::endl;
        return 1;
    }

    //Create Topic
    Topic* PubTopic = PubParticipant->create_topic("samplePubSubTopic", sampleType.get_type_name(), TOPIC_QOS_DEFAULT);

    if (PubTopic == nullptr)
    {
        std::cout << "Something went wrong while creating the Publisher Topic..." << std::endl;
        return 1;
    }

    //Create DataWriter
    DataWriterQos wqos;
    wqos.endpoint().history_memory_policy = DYNAMIC_RESERVE_MEMORY_MODE;

    if (user_configuration.historykind == Keep_Last)
    {
        wqos.history().kind = KEEP_LAST_HISTORY_QOS;
    }
    else
    {
        wqos.history().kind = KEEP_ALL_HISTORY_QOS;
    }

    if (user_configuration.durability == Transient_Local)
    {
        wqos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    }
    else
    {
        wqos.durability().kind = VOLATILE_DURABILITY_QOS;
    }

    if (user_configuration.reliability == Reliable)
    {
        wqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    }
    else
    {
        wqos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;
    }

    wqos.history().depth = user_configuration.depth;
    wqos.resource_limits().max_samples = user_configuration.history_size;
    wqos.resource_limits().max_instances = user_configuration.no_keys;
    wqos.resource_limits().max_samples_per_instance = user_configuration.no_keys > 1 ?
        user_configuration.max_samples_per_key : user_configuration.history_size;

    DataWriter* myWriter = myPub->create_datawriter(PubTopic, wqos);

    if (myWriter == nullptr)
    {
        std::cout << "Something went wrong while creating the Publisher DataWriter..." << std::endl;
        return 1;
    }

    int no_keys = 1;
    sample my_sample;

    std::string c;
    bool condition = true;
    int no;
    while (condition)
    {
        std::cout << "Enter a number to send samples (0 - 255), 'q' to exit" << std::endl;
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
                    myWriter->write(&my_sample);
                }
            }
            std::cout << "Sent " << std::to_string(no) << " samples." << std::endl;
        }
    }

    myPub->delete_datawriter(myWriter);
    PubParticipant->delete_publisher(myPub);
    PubParticipant->delete_topic(PubTopic);
    DomainParticipantFactory::get_instance()->delete_participant(PubParticipant);

    return 0;
}
