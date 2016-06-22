#include <fastrtps/log/StdoutConsumer.h>
#include <fastrtps/log/Colors.h>
#include <iostream>

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
   switch (entry.kind) 
   {
   case Log::Kind::Error:
      std::cout << C_B_RED << "[" << entry.context.category << " - Error] ";
      break;
   case Log::Kind::Warning:
      std::cout << C_B_YELLOW << "[" << entry.context.category << " - Warning] ";
      break;
   case Log::Kind::Info:
      std::cout << C_B_GREEN << "[" << entry.context.category << " - Info] ";
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
