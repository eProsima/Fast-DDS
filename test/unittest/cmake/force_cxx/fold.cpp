// fold.cpp
#include <iostream>

using namespace std;

template<typename ... Args>
void printer(
        Args&&... args)
{
    (cout << ... << args) << endl;
}

int main()
{
    printer("eProsima", 0b101010, 'x');
}
