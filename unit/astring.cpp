#include <gtest/gtest.h>
#include "astring.h"

TEST(astring_char, Basic)
{
	AString test;
	AString test2("test");
	stringstream fake_input("test");
	fake_input >> test;
	EXPECT_EQ(test, test2);
}
