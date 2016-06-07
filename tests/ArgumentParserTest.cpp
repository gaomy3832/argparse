#include "gtest/gtest.h"
#include "argparse/argument_parser.h"

using namespace argparse;

class ArgumentParserTest : public ::testing::Test {
protected:
    virtual void SetUp() {
        ap_ = new ArgumentParser();
    }

    virtual void TearDown() {
        delete ap_;
    }

    ArgumentParser* ap_;
};

TEST_F(ArgumentParserTest, posArg) {
    ap_->argumentNew<int>("a", true, 0, {}, 2, "two numbers");
    ap_->argumentNew<int>("b", true, 0, {}, 1, "a number");
    const char* cmdline[] = {"prog", "7", "5", "3"};
    ap_->cmdlineIs(4, cmdline);
    ASSERT_EQ(2, ap_->argValueCount(0));
    ASSERT_EQ(7, ap_->argValue<int>(0, 0));
    ASSERT_EQ(5, ap_->argValue<int>(0, 1));
    ASSERT_EQ(1, ap_->argValueCount(1));
    ASSERT_EQ(3, ap_->argValue<int>(1));
}

TEST_F(ArgumentParserTest, posArgDefault) {
    ap_->argumentNew<int>("a", false, 9, {}, 2, "two numbers");
    const char* cmdline[] = {"prog"};
    ap_->cmdlineIs(1, cmdline);
    ASSERT_EQ(2, ap_->argValueCount(0));
    ASSERT_EQ(9, ap_->argValue<int>(0, 0));
    ASSERT_EQ(9, ap_->argValue<int>(0, 1));
}

TEST_F(ArgumentParserTest, posArgRequiredNotGiven) {
    ap_->argumentNew<int>("a", true, 0, {}, 1, "a number");
    const char* cmdline[] = {"prog"};
    try {
        ap_->cmdlineIs(1, cmdline);
    } catch (ArgKeyException& e) {
        ASSERT_EQ("a", std::string(e.key()));
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST_F(ArgumentParserTest, posArgRequiredNotEnough) {
    ap_->argumentNew<int>("a", true, 0, {}, 4, "four numbers");
    const char* cmdline[] = {"prog", "2"};
    try {
        ap_->cmdlineIs(2, cmdline);
    } catch (ArgKeyException& e) {
        ASSERT_EQ("a", std::string(e.key()));
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST_F(ArgumentParserTest, posArgNotChoice) {
    ap_->argumentNew<int>("a", true, 0, {0, 1}, 1, "a number");
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
        ap_->argumentNew<int>("a", false, 2, {0, 1}, 1, "a number");
    } catch (ArgPropertyException& e) {
        ASSERT_EQ("defaultValue", std::string(e.property()));
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST_F(ArgumentParserTest, posArgNonRequiredPosition) {
    ap_->argumentNew<int>("a", false, 0, {}, 1, "a number");
    try {
        ap_->argumentNew<int>("b", true, 0, {}, 1, "a number");
    } catch (ArgPropertyException& e) {
        ASSERT_EQ("required", std::string(e.property()));
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST_F(ArgumentParserTest, posArgZeroExpectCount) {
    try {
        ap_->argumentNew<int>("a", true, 0, {}, 0, "zero number");
    } catch (ArgPropertyException& e) {
        ASSERT_EQ("expectCount", std::string(e.property()));
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST_F(ArgumentParserTest, posArgUnlimitedExpectCount) {
    try {
        ap_->argumentNew<int>("a", true, 0, {}, -1, "any count of numbers");
    } catch (ArgPropertyException& e) {
        ASSERT_EQ("expectCount", std::string(e.property()));
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST_F(ArgumentParserTest, posArgTooMany) {
    ap_->argumentNew<int>("a", true, 0, {}, 2, "two numbers");
    ap_->argumentNew<int>("b", true, 0, {}, 1, "a number");
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

