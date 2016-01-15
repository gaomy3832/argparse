#include "gtest/gtest.h"
#include "argument_parser.h"

using namespace argparse;

TEST(ArgValueTest, string) {
    ArgValue av("foo");
    ASSERT_EQ("foo", av.value());
}

