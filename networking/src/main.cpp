#include "util/member_function.hpp"
#include <functional>
#include <iostream>
// #include "util/named_function.hpp"
// #include "util/event.hpp"

using namespace util;

void f0(int arg)
{
	std::cout << "[0] hewwo I am " << arg << "\n";
}

void f1(int arg)
{
	std::cout << "[1] hewwo I am " << arg << "\n";
}

void f2(int arg)
{
	std::cout << "[2] hewwo I am " << arg << "\n";
}

class TestClass
{
private:
	int m_first;

	void print_int(int i)
	{
		std::cout << "number " << (m_first++ + i) << "!\n";
	}

public:
	TestClass(int first)
	{
		m_first = first;
	}

	MemberFunction<void(int)> member_printer()
	{
		return MemberFunction<void(int)>(&TestClass::print_int, this);
	}
};

int main(int argc, char **argv)
{
	TestClass t0 = TestClass(5);
	TestClass t1 = TestClass(8);

	MemberFunction<void(int)> p0 = t0.member_printer();
	MemberFunction<void(int)> p1 = t1.member_printer();
	MemberFunction<void(int)> p2 = t0.member_printer();

	std::cout << std::hash<MemberFunction<void(int)>>{}(p0) << "\n";
	std::cout << std::hash<MemberFunction<void(int)>>{}(p1) << "\n";
	std::cout << std::hash<MemberFunction<void(int)>>{}(p2) << "\n";

	p0(1);
	p1(1);
	p0(1);

	std::cout << &p0 << "\n";
	std::cout << &p1 << "\n";
	std::cout << &p2 << "\n";




	// std::function<void(int)> printer = std::mem_fn(&TestClass::print_int);

	// const NamedFunction<void(int)> nf0(f0, "f0");
	// NamedFunction<void(int)> nf1(f1, "f1");
	// NamedFunction<void(int)> nf2(f2, "f2");
	//
	// Event<int> event;
	//
	// event.subscribe(nf0);
	// event.subscribe(nf1);
	// event.subscribe(nf2);
	//
	// event.invoke(6);
	//
	// event.unsubscribe(nf1);
	// event.unsubscribe(nf0);
	// event.unsubscribe(nf2);
	// event.subscribe(nf0);
	// event.subscribe(nf1);
	// event.subscribe(nf2);
	//
	// event.invoke(2);
	//
	// // event += nf0;
	// // event += nf1;
	// // event += nf2;
	//
	// std::cout << "Hello World!\n";
	return 0;
}
