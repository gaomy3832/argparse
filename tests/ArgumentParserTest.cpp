/*
 * Copyright 2016 Mingyu Gao
 *
 */
#include "gtest/gtest.h"
#include <cmath>
#include "argparse/argument_parser.h"

using namespace argparse;

// ASSERT_EQ for floating point types.
template<typename T, typename = typename std::enable_if<std::is_floating_point<T>::value>::type>
inline static void ASSERT_EQ_WithError(const T a, const T b, const T error) {
    ASSERT_LT(std::abs(a - b), error);
}

class ArgumentParserTest : public ::testing::Test {
protected:
    virtual void SetUp() {
        ap_ = new ArgumentParser("UnitTest");
    }

    virtual void TearDown() {
        delete ap_;
    }

    ArgumentParser* ap_;
};

TEST_F(ArgumentParserTest, argValueposArgBadKey) {
    try {
        ap_->argValue(0);
    } catch (ArgKeyException& e) {
        ASSERT_EQ("@0", std::string(e.key()));
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST_F(ArgumentParserTest, argValueOptionBadKey) {
    try {
        ap_->argValue("-a");
    } catch (ArgKeyException& e) {
        ASSERT_EQ("-a", std::string(e.key()));
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST_F(ArgumentParserTest, argValueOutOfIndex) {
    ap_->argumentNew<std::string>("a", "two strings", 2);
    const char* cmdline[] = {"prog", "123", "234"};
    ap_->cmdlineIs(3, cmdline);

    ASSERT_EQ("123", ap_->argValue(0, 0));
    ASSERT_EQ("234", ap_->argValue(0, 1));
    ASSERT_EQ("", ap_->argValue(0, 2));
}

TEST_F(ArgumentParserTest, posArg) {
    ap_->argumentNew<int>("a", "two numbers", 2);
    ap_->argumentNew<int>("b", "a number", 1);
    const char* cmdline[] = {"prog", "7", "5", "3"};
    ap_->cmdlineIs(4, cmdline);
    ASSERT_EQ(2, ap_->argValueCount(0));
    ASSERT_EQ(7, ap_->argValue<int>(0, 0));
    ASSERT_EQ(5, ap_->argValue<int>(0, 1));
    ASSERT_EQ(1, ap_->argValueCount(1));
    ASSERT_EQ(3, ap_->argValue<int>(1));
}

TEST_F(ArgumentParserTest, posArgDefault) {
    ap_->argumentNew<int>("a", "two numbers", 2, false, 9);
    const char* cmdline[] = {"prog"};
    ap_->cmdlineIs(1, cmdline);
    ASSERT_EQ(2, ap_->argValueCount(0));
    ASSERT_EQ(9, ap_->argValue<int>(0, 0));
    ASSERT_EQ(9, ap_->argValue<int>(0, 1));
}

TEST_F(ArgumentParserTest, posArgPartialDefault) {
    ap_->argumentNew<int>("a", "two numbers", 2, false, 9);
    const char* cmdline[] = {"prog", "3"};
    ap_->cmdlineIs(2, cmdline);
    ASSERT_EQ(2, ap_->argValueCount(0));
    ASSERT_EQ(3, ap_->argValue<int>(0, 0));
    ASSERT_EQ(9, ap_->argValue<int>(0, 1));
}

TEST_F(ArgumentParserTest, posArgRequiredNotGiven) {
    ap_->argumentNew<int>("a", "a number", 1);
    const char* cmdline[] = {"prog"};
    try {
        ap_->cmdlineIs(1, cmdline);
    } catch (ArgPropertyException& e) {
        ASSERT_EQ("a", std::string(e.key()));
        ASSERT_EQ("required", std::string(e.property()));
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST_F(ArgumentParserTest, posArgRequiredNotEnough) {
    ap_->argumentNew<int>("a", "four numbers", 4);
    const char* cmdline[] = {"prog", "2"};
    try {
        ap_->cmdlineIs(2, cmdline);
    } catch (ArgPropertyException& e) {
        ASSERT_EQ("a", std::string(e.key()));
        ASSERT_EQ("expectCount", std::string(e.property()));
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST_F(ArgumentParserTest, posArgNotChoice) {
    ap_->argumentNew<int>("a", "a number", 1, true, 0, {0, 1});
    const char* cmdline[] = {"prog", "2"};
    try {
        ap_->cmdlineIs(2, cmdline);
    } catch (ArgValueException& e) {
        ASSERT_EQ("2", std::string(e.val()));
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST_F(ArgumentParserTest, posArgDefaultNotChoice) {
    try {
        ap_->argumentNew<int>("a", "a number", 1, false, 2, {0, 1});
    } catch (ArgPropertyException& e) {
        ASSERT_EQ("defaultValue", std::string(e.property()));
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST_F(ArgumentParserTest, posArgNonRequiredPosition) {
    ap_->argumentNew<int>("a", "a number", 1, false);
    try {
        ap_->argumentNew<int>("b", "a number", 1);
    } catch (ArgPropertyException& e) {
        ASSERT_EQ("required", std::string(e.property()));
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST_F(ArgumentParserTest, posArgZeroExpectCount) {
    try {
        ap_->argumentNew<int>("a", "zero number", 0);
    } catch (ArgPropertyException& e) {
        ASSERT_EQ("expectCount", std::string(e.property()));
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST_F(ArgumentParserTest, posArgUnlimitedExpectCount) {
    try {
        ap_->argumentNew<int>("a", "any count of numbers", -1);
    } catch (ArgPropertyException& e) {
        ASSERT_EQ("expectCount", std::string(e.property()));
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST_F(ArgumentParserTest, posArgTooMany) {
    ap_->argumentNew<int>("a", "two numbers", 2);
    ap_->argumentNew<int>("b", "a number", 1);
    const char* cmdline[] = {"prog", "7", "5", "3", "4"};
    try {
        ap_->cmdlineIs(5, cmdline);
    } catch (ArgKeyException& e) {
        ASSERT_EQ("@2", std::string(e.key()));
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST_F(ArgumentParserTest, option) {
    ap_->argumentNew<int>("--a", "two numbers", 2);
    ap_->argumentNew<int>("-b", "a number", 1);
    ap_->argumentNew<int>("--cc", "an optional number", 1, false, 9);
    ap_->argumentNew<int>("-f", "a flag", 0, false);
    ap_->argumentNew<int>("-n", "a number", 1, true, 0, {}, {"--nn", "--nnn"});
    const char* cmdline[] = {"prog", "--a", "2", "4", "-f", "-b", "5", "--nnn", "8"};
    ap_->cmdlineIs(9, cmdline);

    ASSERT_EQ(2, ap_->argValueCount("--a"));
    ASSERT_TRUE(ap_->optionGiven("--a"));
    ASSERT_EQ(2, ap_->argValue<int>("--a", 0));
    ASSERT_EQ(4, ap_->argValue<int>("--a", 1));

    ASSERT_EQ(1, ap_->argValueCount("-b"));
    ASSERT_TRUE(ap_->optionGiven("-b"));
    ASSERT_EQ(5, ap_->argValue<int>("-b"));

    ASSERT_EQ(1, ap_->argValueCount("--cc"));
    ASSERT_FALSE(ap_->optionGiven("--cc"));
    ASSERT_EQ(9, ap_->argValue<int>("--cc"));

    ASSERT_EQ(0, ap_->argValueCount("-f"));
    ASSERT_TRUE(ap_->optionGiven("-f"));

    ASSERT_EQ(1, ap_->argValueCount("-n"));
    ASSERT_TRUE(ap_->optionGiven("-n"));
    ASSERT_EQ(8, ap_->argValue<int>("-n"));
}

TEST_F(ArgumentParserTest, optionPartialDefault) {
    ap_->argumentNew<int>("--cc", "three optional numbers", 3, false, 9);
    const char* cmdline[] = {"prog", "--cc", "2", "4"};
    ap_->cmdlineIs(4, cmdline);

    ASSERT_EQ(3, ap_->argValueCount("--cc"));
    ASSERT_TRUE(ap_->optionGiven("--cc"));
    ASSERT_EQ(2, ap_->argValue<int>("--cc", 0));
    ASSERT_EQ(4, ap_->argValue<int>("--cc", 1));
    ASSERT_EQ(9, ap_->argValue<int>("--cc", 2));
}

TEST_F(ArgumentParserTest, optionUnlimitedExpectCount) {
    ap_->argumentNew<int>("--cc", "any number of numbers", -1);
    const char* cmdline[] = {"prog", "--cc", "2", "4", "5"};
    ap_->cmdlineIs(5, cmdline);

    ASSERT_EQ(3, ap_->argValueCount("--cc"));
    ASSERT_TRUE(ap_->optionGiven("--cc"));
    ASSERT_EQ(2, ap_->argValue<int>("--cc", 0));
    ASSERT_EQ(4, ap_->argValue<int>("--cc", 1));
    ASSERT_EQ(5, ap_->argValue<int>("--cc", 2));
}

TEST_F(ArgumentParserTest, optionUnlimitedExpectCountRequiredNotGiven) {
    ap_->argumentNew<int>("--cc", "any number of numbers", -1);
    const char* cmdline[] = {"prog"};
    try {
        ap_->cmdlineIs(1, cmdline);
    } catch (ArgPropertyException& e) {
        ASSERT_EQ("--cc", std::string(e.key()));
        ASSERT_EQ("required", std::string(e.property()));
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST_F(ArgumentParserTest, optionUnlimitedExpectCountNotRequiredNotGiven) {
    ap_->argumentNew<int>("--cc", "any number of numbers", -1, false);
    const char* cmdline[] = {"prog"};
    ap_->cmdlineIs(1, cmdline);

    ASSERT_EQ(0, ap_->argValueCount("--cc"));
}

TEST_F(ArgumentParserTest, optionRequiredNotGiven) {
    ap_->argumentNew<int>("-a", "a number", 1);
    const char* cmdline[] = {"prog"};
    try {
        ap_->cmdlineIs(1, cmdline);
    } catch (ArgPropertyException& e) {
        ASSERT_EQ("-a", std::string(e.key()));
        ASSERT_EQ("required", std::string(e.property()));
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST_F(ArgumentParserTest, optionRequiredNotEnough) {
    ap_->argumentNew<int>("-a", "four numbers", 4);
    const char* cmdline[] = {"prog", "-a", "2"};
    try {
        ap_->cmdlineIs(3, cmdline);
    } catch (ArgPropertyException& e) {
        ASSERT_EQ("-a", std::string(e.key()));
        ASSERT_EQ("expectCount", std::string(e.property()));
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST_F(ArgumentParserTest, optionNotChoice) {
    ap_->argumentNew<int>("-a", "a number", 1, true, 0, {0, 1});
    const char* cmdline[] = {"prog", "-a", "2"};
    try {
        ap_->cmdlineIs(3, cmdline);
    } catch (ArgValueException& e) {
        ASSERT_EQ("2", std::string(e.val()));
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST_F(ArgumentParserTest, optionDefaultNotChoice) {
    try {
        ap_->argumentNew<int>("-a", "a number", 1, false, 2, {0, 1});
    } catch (ArgPropertyException& e) {
        ASSERT_EQ("defaultValue", std::string(e.property()));
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST_F(ArgumentParserTest, optionRequiredPureFlag) {
    try {
        ap_->argumentNew<int>("-a", "a flag", 0);
    } catch (ArgPropertyException& e) {
        ASSERT_EQ("required", std::string(e.property()));
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST_F(ArgumentParserTest, optionNotSeen) {
    ap_->argumentNew<int>("-a", "two numbers", 2);
    const char* cmdline[] = {"prog", "-b", "7", "5"};
    try {
        ap_->cmdlineIs(4, cmdline);
    } catch (ArgKeyException& e) {
        ASSERT_EQ("-b", std::string(e.key()));
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST_F(ArgumentParserTest, optionAliasNotFlag) {
    try {
        ap_->argumentNew<int>("-n", "a number", 1, true, 0, {}, {"nn"});
    } catch (ArgPropertyException& e) {
        ASSERT_EQ("alias", std::string(e.property()));
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST_F(ArgumentParserTest, cmdlineIs) {
    ap_->argumentNew<int>("a", "two integers", 2);
    ap_->argumentNew<std::string>("b", "a string", 1);
    ap_->argumentNew<double>("-c", "a double number", 1);
    ap_->argumentNew<uint64_t>("--dd", "an optional number", 1, false, 9);
    ap_->argumentNew<std::string>("-f", "a flag", 0, false);

    // Const argv, interleaved pos arg and options.
    const char* cmdline1[] = {"prog", "-f", "7", "-5", "--dd", "100", "D", "-c", "-1.5"};
    ap_->cmdlineIs(9, cmdline1);

    ASSERT_EQ(2, ap_->argValueCount(0));
    ASSERT_EQ(7, ap_->argValue<int>(0, 0));
    ASSERT_EQ(-5, ap_->argValue<int>(0, 1));

    ASSERT_EQ(1, ap_->argValueCount(1));
    ASSERT_EQ("D", ap_->argValue(1));

    ASSERT_EQ(1, ap_->argValueCount("-c"));
    ASSERT_EQ_WithError(-1.5, ap_->argValue<double>("-c"), 1e-4);

    ASSERT_TRUE(ap_->optionGiven("--dd"));
    ASSERT_EQ(1, ap_->argValueCount("--dd"));
    ASSERT_EQ(100, ap_->argValue<uint64_t>("--dd"));

    ASSERT_TRUE(ap_->optionGiven("-f"));

    // Non-const argv.
    char argv0[] = "prog";
    char argv1[] = "-7";
    char argv2[] = "5";
    char argv3[] = "B";
    char argv4[] = "-c";
    char argv5[] = "1.5";
    char* cmdline2[] = {argv0, argv1, argv2, argv3, argv4, argv5};
    ap_->cmdlineIs(6, cmdline2);

    ASSERT_EQ(2, ap_->argValueCount(0));
    ASSERT_EQ(-7, ap_->argValue<int>(0, 0));
    ASSERT_EQ(5, ap_->argValue<int>(0, 1));

    ASSERT_EQ(1, ap_->argValueCount(1));
    ASSERT_EQ("B", ap_->argValue(1));

    ASSERT_TRUE(ap_->optionGiven("-c"));
    ASSERT_EQ(1, ap_->argValueCount("-c"));
    ASSERT_EQ_WithError(1.5, ap_->argValue<double>("-c"), 1e-4);

    ASSERT_FALSE(ap_->optionGiven("--dd"));

    ASSERT_FALSE(ap_->optionGiven("-f"));
}

TEST_F(ArgumentParserTest, cmdlineIsOptionInMiddleOfPosArg) {
    ap_->argumentNew<int>("a", "two integers", 2);
    ap_->argumentNew<std::string>("b", "a string", 1);
    ap_->argumentNew<std::string>("-f", "a flag", 0, false);
    const char* cmdline1[] = {"prog", "7", "-f", "-5", "B"};
    try {
        ap_->cmdlineIs(4, cmdline1);
    } catch (ArgPropertyException& e) {
        ASSERT_EQ("a", std::string(e.key()));
        ASSERT_EQ("expectCount", std::string(e.property()));
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

