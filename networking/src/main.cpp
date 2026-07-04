#include "util/event.hpp"
#include "util/member_function.hpp"
#include <iostream>

using namespace util;

class TestClass
{
private:
	Event<int>& m_event;
	int m_first;

	void print_int(int i)
	{
		std::cout << "number " << (m_first++ + i) << "!\n";
	}

public:
	TestClass(int first, Event<int>& event) :
		m_event(event),
		m_first(first)
	{
		m_event += EventFunction<int>(&TestClass::print_int, this);
	}
};

int main(int argc, char **argv)
{
	Event<int> event;

	TestClass t0 = TestClass(5, event);
	TestClass t1 = TestClass(8, event);

	event.invoke(6);
	event.invoke(6);

	return 0;
}
