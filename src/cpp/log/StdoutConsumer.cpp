#include <fastrtps/log/StdoutConsumer.h>
#include <fastrtps/log/Colors.h>
#include <iostream>
#include <iomanip>

namespace eprosima {
namespace fastrtps {

void StdoutConsumer::Consume(const Log::Entry& entry)
{
   PrintHeader(entry);
   std::cout << C_WHITE << entry.message;
   PrintContext(entry);
   std::cout << C_DEF << std::endl;
}

void StdoutConsumer::PrintHeader(const Log::Entry& entry) const
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::chrono::system_clock::duration tp = now.time_since_epoch();
    tp -= std::chrono::duration_cast<std::chrono::seconds>(tp);
    unsigned ms = static_cast<unsigned>(tp / std::chrono::milliseconds(1));
#ifdef _WIN32
    std::tm ltime;
    localtime_s(&ltime, &now_c);
    std::cout << C_B_WHITE << std::put_time(&ltime, "%F %T")
#else
    std::cout << C_B_WHITE << std::put_time(localtime(&now_c), "%F %T")
#endif
       << "." << std::setw(3) << std::setfill('0') <<  ms << " ";
    switch (entry.kind)
    {
    case Log::Kind::Error:
        std::cout << C_B_RED << "[" << C_B_WHITE << entry.context.category << C_B_RED << " Error] ";
        break;
    case Log::Kind::Warning:
        std::cout << C_B_YELLOW << "[" << C_B_WHITE << entry.context.category << C_B_YELLOW << " Warning] ";
        break;
    case Log::Kind::Info:
        std::cout << C_B_GREEN << "[" << C_B_WHITE << entry.context.category << C_B_GREEN << " Info] ";
        break;
    }
}

void StdoutConsumer::PrintContext(const Log::Entry& entry) const
{
   std::cout << C_B_BLUE;
    if (entry.context.filename)
    {
      std::cout << " (" << entry.context.filename;
      std::cout << ":" << entry.context.line << ")";
    }
    if (entry.context.function)
    {
      std::cout << " -> Function " << C_CYAN << entry.context.function;
    }
}

} // Namespace fastrtps
} // Namespace eprosima
