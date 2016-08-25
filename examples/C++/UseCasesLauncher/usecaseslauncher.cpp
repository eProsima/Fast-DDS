/** Header pending 
 *
 *
 * This file contains the launcher for an example
 *
 * */

#include <iostream>
#include <string>
#include <memory>


int main(int argc, char** argv){

    std::cout << "eProsima Fast RTPS Use Case Launcher" << std::endl;
    std::cout << "------------------------------------" << std::endl;


    if(argc <= 1){
    
        std::cout << "Usage: \"usecaselauncher <scenario>\". The possible scenarios are:" << std::endl;
        std::cout << "  videostream - a data intensive application where having the latest frame is more important than having all the frames" << std::endl;
        std::cout << "  audiostream - a data intensive application where we do need all the samples, but lost samples can be recovered algorithmically" << std::endl;
        std::cout << "  signalfilter - a filter needs acces to previous samples in order to calculate its output. Missing samples could break the filter" << std::endl;
        std::cout << "  alarms - an alarm is an event that cannot be missed" << std::endl;
        std::cout << "  multiplesensors - defining keys on a topic enables the user to add sensors to an existing topic without modifying the network" << std::endl;
        std::cout << "Refer to the README for a detailed description of each of these use cases" << std::endl;
        return 1;
    }
    
    if(std::string(argv[1])=="videostream"){
        std::cout << "Running the video streaming use case..." << std::endl;         

    }
    if(std::string(argv[1])=="audiostream"){
        std::cout << "Running the audio streaming use case..." << std::endl;

    }
    if(std::string(argv[1])=="signalfilter"){
        std::cout << "Running the signal filter use case..." << std::endl;

    }
    if(std::string(argv[1])=="alarms"){
        std::cout << "Running the alarms use case..." << std::endl;

    }
    if(std::string(argv[1])=="multiplesensors"){
        std::cout << "Running the multiple sensors use case..." << std::endl;

    }

    return 0;
}


