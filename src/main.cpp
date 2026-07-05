#include "events/event.hpp"
#include <iostream>

using namespace fluffy_fox;

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
		m_event.subscribe(&TestClass::print_int, this);
	}

	void leave()
	{
		m_event.unsubscribe(&TestClass::print_int, this);
	}
};

int main(int argc, char **argv)
{
	Event<int> event;

	TestClass t0 = TestClass(5, event);
	TestClass t1 = TestClass(8, event);
	TestClass t2 = TestClass(8, event);

	event.invoke(6);
	t0.leave();
	event.invoke(6);

	return 0;
}
