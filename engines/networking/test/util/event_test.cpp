#include "doctest.hpp"
#include "util/event.hpp"

void succ(int* i_ptr)
{
	*i_ptr += 1;
}

void double_val(int* i_ptr)
{
	*i_ptr *= 2;
}

void add_5(int* i_ptr)
{
	*i_ptr += 5;
}

void add_numbers(int a, int b, int* out)
{
	*out = a + b;
}

TEST_CASE("should call function")
{
	// Arrange
	NamedFunction<void(int*)> nf0(succ, "f0");
	NamedFunction<void(int*)> nf1(succ, "f1");
	NamedFunction<void(int*)> nf2(succ, "f2");

	Event<int*> event;

	event.subscribe(nf0);
	event.subscribe(nf1);
	event.subscribe(nf2);

	int i0 = 4;

	// Act
	event.invoke(&i0);

	// Assert
	CHECK(i0 == 7);
}

TEST_CASE("should not throw when empty")
{
	// Arrange
	Event<int*> event;
	bool exception_ocurred = false;

	// Act
	try
	{
		int i = 0;
		event.invoke(&i);
	}
	catch (std::exception e)
	{
		exception_ocurred = true;
	}

	// Assert
	CHECK_FALSE(exception_ocurred);
}

TEST_CASE("should unsubscribe")
{
	// Arrange
	NamedFunction<void(int*)> nf0(succ, "f0");
	NamedFunction<void(int*)> nf1(succ, "f1");
	NamedFunction<void(int*)> nf2(succ, "f2");

	Event<int*> event;

	event.subscribe(nf0);
	event.subscribe(nf1);
	event.subscribe(nf2);
	event.unsubscribe(nf0);

	int i0 = 4;

	// Act
	event.invoke(&i0);

	// Assert
	CHECK(i0 == 6);
}

TEST_CASE("should resubscribe correctly")
{
	// Arrange
	NamedFunction<void(int*)> nf0(succ, "f0");
	NamedFunction<void(int*)> nf1(succ, "f1");
	NamedFunction<void(int*)> nf2(succ, "f2");

	Event<int*> event;

	event.subscribe(nf0);
	event.subscribe(nf1);
	event.subscribe(nf2);
	event.unsubscribe(nf0);
	event.subscribe(nf0);

	int i0 = 4;

	// Act
	event.invoke(&i0);

	// Assert
	CHECK(i0 == 7);
}

TEST_CASE("should not throw when unsubscribing nonexistant function")
{
	// Arrange
	NamedFunction<void(int*)> nf0(succ, "f0");
	NamedFunction<void(int*)> nf1(succ, "f1");

	Event<int*> event;
	event.subscribe(nf0);

	bool exception_ocurred = false;
	int i0 = 4;

	// Act
	try
	{
		event.unsubscribe(nf1);
		event.invoke(&i0);
	}
	catch (std::exception e)
	{
		exception_ocurred = true;
	}

	// Assert
	CHECK_FALSE(exception_ocurred);
	CHECK(i0 == 5);
}

TEST_CASE("should not oversubscribe when subscribing existing function")
{
	// Arrange
	NamedFunction<void(int*)> nf0(succ, "f0");

	Event<int*> event;
	event.subscribe(nf0);

	bool exception_ocurred = false;
	int i0 = 4;

	// Act
	try
	{
		event.subscribe(nf0);
		event.invoke(&i0);
	}
	catch (std::exception e)
	{
		exception_ocurred = true;
	}

	// Assert
	CHECK_FALSE(exception_ocurred);
	CHECK(i0 == 5);
}

TEST_CASE("should work with different functions")
{
	// Arrange
	NamedFunction<void(int*)> nsucc(succ, "f0");
	NamedFunction<void(int*)> ndouble_val(double_val, "f1");
	NamedFunction<void(int*)> nadd_5(add_5, "f2");

	Event<int*> event;

	event.subscribe(nsucc);
	event.subscribe(ndouble_val);
	event.subscribe(nadd_5);

	int i0 = 4;

	// Act
	event.invoke(&i0);

	// Assert
	CHECK(i0 == 15);
}

TEST_CASE("should work with several arguments")
{
	// Arrange
	NamedFunction<void(int,int,int*)> nadd_numbers(add_numbers, "f0");

	Event<int,int,int*> event;

	event.subscribe(nadd_numbers);

	int i0 = 4;
	int i1 = 6;
	int i2 = 0;

	// Act
	event.invoke(i0, i1, &i2);

	// Assert
	CHECK(i2 == 10);
}
