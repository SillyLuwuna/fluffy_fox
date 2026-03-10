#include <iostream>
#include "util/named_function.hpp"
#include "util/delegate.hpp"

using namespace util;

bool f0(int arg)
{
	std::cout << "[0] hewwo I am " << arg << "\n";
	return true;
}

bool f1(int arg)
{
	std::cout << "[1] hewwo I am " << arg << "\n";
	return true;
}

bool f2(int arg)
{
	std::cout << "[2] hewwo I am " << arg << "\n";
	return true;
}

int main(int argc, char **argv)
{
	NamedFunction<bool(int)> nf0(f0, "f0");
	NamedFunction<bool(int)> nf1(f1, "f1");
	NamedFunction<bool(int)> nf2(f2, "f2");

	Delegate<bool, int> event;

	event.subscribe(nf0);
	event.subscribe(nf1);
	event.subscribe(nf2);

	event.invoke(6);

	event.unsubscribe(nf1);
	event.unsubscribe(nf0);
	event.unsubscribe(nf2);
	event.subscribe(nf0);
	event.subscribe(nf1);
	event.subscribe(nf2);

	event.invoke(2);

	// event += nf0;
	// event += nf1;
	// event += nf2;

	std::cout << "Hello World!\n";
	return 0;
}
