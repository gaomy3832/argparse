#include "gtest/gtest.h"
#include "argument_parser.h"

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
    ap_->argumentNew<int>("a", true, 0, {}, "two numbers", 2, {});
    ap_->argumentNew<int>("b", true, 0, {}, "a number", 1, {});
    const char* cmdline[] = {"prog", "7", "5", "3"};
    ap_->cmdlineIs(4, cmdline);
    ASSERT_EQ(2, ap_->argValueCount(0));
    ASSERT_EQ(7, ap_->argValue<int>(0, 0));
    ASSERT_EQ(5, ap_->argValue<int>(0, 1));
    ASSERT_EQ(1, ap_->argValueCount(1));
    ASSERT_EQ(3, ap_->argValue<int>(1));
}

TEST_F(ArgumentParserTest, posArgDefault) {
    ap_->argumentNew<int>("a", false, 9, {}, "two numbers", 2, {});
    const char* cmdline[] = {"prog"};
    ap_->cmdlineIs(1, cmdline);
    ASSERT_EQ(2, ap_->argValueCount(0));
    ASSERT_EQ(9, ap_->argValue<int>(0, 0));
    ASSERT_EQ(9, ap_->argValue<int>(0, 1));
}

TEST_F(ArgumentParserTest, posArgRequiredNotGiven) {
    ap_->argumentNew<int>("a", true, 0, {}, "a number", 1, {});
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
    ap_->argumentNew<int>("a", true, 0, {}, "four numbers", 4, {});
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
    ap_->argumentNew<int>("a", true, 0, {0, 1}, "a number", 1, {});
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
        ap_->argumentNew<int>("a", false, 2, {0, 1}, "a number", 1, {});
    } catch (ArgPropertyException& e) {
        ASSERT_EQ("defaultValue", std::string(e.property()));
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST_F(ArgumentParserTest, posArgNonRequiredPosition) {
    ap_->argumentNew<int>("a", false, 0, {}, "a number", 1, {});
    try {
        ap_->argumentNew<int>("b", true, 0, {}, "a number", 1, {});
    } catch (ArgPropertyException& e) {
        ASSERT_EQ("required", std::string(e.property()));
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

