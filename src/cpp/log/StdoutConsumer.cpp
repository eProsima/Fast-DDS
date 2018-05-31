#include <fastrtps/log/StdoutConsumer.h>
#include <fastrtps/log/Colors.h>
#include <iostream>

namespace eprosima {
namespace fastrtps {

std::recursive_mutex StdoutConsumer::stdOutMutex;

void StdoutConsumer::Consume(const Log::Entry& entry) 
{
    std::stringstream ss;
    PrintHeader(ss, entry);
    ss << C_WHITE << entry.message;
    PrintContext(ss, entry);

    {
        std::lock_guard<std::recursive_mutex> scoped(stdOutMutex);
        std::cout << ss.str() << C_DEF << std::endl;
    }
}

void StdoutConsumer::PrintHeader(std::stringstream &ss, const Log::Entry& entry) const
{
    switch (entry.kind) 
    {
    case Log::Kind::Error:
        ss << C_B_RED << "[" << C_B_WHITE << entry.context.category << C_B_RED << " Error] ";
        break;
    case Log::Kind::Warning:
        ss << C_B_YELLOW << "[" << C_B_WHITE << entry.context.category << C_B_YELLOW << " Warning] ";
        break;
    case Log::Kind::Info:
        ss << C_B_GREEN << "[" << C_B_WHITE << entry.context.category << C_B_GREEN << " Info] ";
        break;
    }
}

void StdoutConsumer::PrintContext(std::stringstream &ss, const Log::Entry& entry) const
{ 
    ss << C_B_BLUE;
    if (entry.context.filename) 
    {
        ss << " (" << entry.context.filename;
        ss << ":" << entry.context.line << ")";
    }
    if (entry.context.function)
    {
        ss << " -> Function " << C_CYAN << entry.context.function;
    }
}

} // Namespace fastrtps
} // Namespace eprosima
