#include "doctest.hpp"
#include "util/delegate.hpp"

void succ(int* i_ptr)
{
	*i_ptr += 1;
}

void double_val(int* i_ptr)
{
	*i_ptr *= (*i_ptr);
}

void add_5(int* i_ptr)
{
	*i_ptr += 5;
}

TEST_CASE("should call function")
{
	// Arrange
	NamedFunction<void(int*)> nf0(succ, "f0");
	NamedFunction<void(int*)> nf1(succ, "f1");
	NamedFunction<void(int*)> nf2(succ, "f2");

	Delegate<int*> event;

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
	Delegate<int*> event;
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

	Delegate<int*> event;

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

	Delegate<int*> event;

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

	Delegate<int*> event;
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

	Delegate<int*> event;
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

}
