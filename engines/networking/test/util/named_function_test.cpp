#include "doctest.hpp"
#include "util/named_function.hpp"

int succ(int i)
{
	return i + 1;
}

void nothing()
{
}

int add(int x, int y)
{
	return x + y;
}

TEST_CASE("should call function")
{
	// Arrange
	NamedFunction<int(int)> nf(succ, "name");
	int i0 = 2;
	int i1 = 6;
	int i2 = -2;
	int expected0 = 3;
	int expected1 = 7;
	int expected2 = -1;

	// Act
	int gotten0 = nf(i0);
	int gotten1 = nf(i1);
	int gotten2 = nf(i2);

	// Assert
	CHECK(gotten0 == expected0);
	CHECK(gotten1 == expected1);
	CHECK(gotten2 == expected2);
}

TEST_CASE("should get name")
{
	// Arrange
	NamedFunction<int(int)> nf0(succ, "owo");
	NamedFunction<int(int)> nf1(succ, "uwu");
	std::string expected0 = "owo";
	std::string expected1 = "uwu";

	// Act
	std::string gotten0 = nf0.get_name();
	std::string gotten1 = nf1.get_name();

	// Assert
	CHECK(gotten0 == expected0);
	CHECK(gotten1 == expected1);
}

TEST_CASE("should work with void")
{
	// Arrange
	bool exception_ocurred = false;

	// Act
	try
	{
		NamedFunction<void(void)> nf0(nothing, "owo");
		nf0();
	}
	catch (std::exception e)
	{
		exception_ocurred = true;
	}

	// Assert
	CHECK_FALSE(exception_ocurred);
}

TEST_CASE("should work with several arguments")
{
	// Arrange
	NamedFunction<int(int, int)> nf0(add, "owo");
	NamedFunction<int(int, int)> nf1(add, "uwu");
	int expected0 = 5;
	int expected1 = -8;

	// Act
	int gotten0 = nf0(2, 3);
	int gotten1 = nf1(8, -16);

	// Assert
	CHECK(gotten0 == expected0);
	CHECK(gotten1 == expected1);
}
