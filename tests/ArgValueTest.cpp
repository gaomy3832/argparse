#include "gtest/gtest.h"
#include <limits>
#include "argument_parser.h"

using namespace argparse;

// ASSERT_EQ for floating point types.
template<typename T, typename = typename std::enable_if<std::is_floating_point<T>::value>::type>
inline static void ASSERT_EQ_WithError(const T a, const T b, const T error) {
    ASSERT_LT(std::abs(a - b), error);
}

TEST(ArgValueTest, string) {
    ArgValue av("foo");
    ASSERT_EQ("foo", av.value());
}

TEST(ArgValueTest, uint64) {
    ArgValue av("1234");
    ASSERT_EQ(1234uLL, av.value<uint64_t>());
    av = ArgValue("18446744073709551615");
    ASSERT_EQ(18446744073709551615uLL, av.value<uint64_t>());
}

TEST(ArgValueTest, uint64BadCharBefore) {
    try {
        ArgValue av("x1234");
        av.value<uint64_t>();
    } catch (ArgTypeException& e) {
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST(ArgValueTest, uint64BadCharAfter) {
    try {
        ArgValue av("1234x");
        av.value<uint64_t>();
    } catch (ArgTypeException& e) {
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST(ArgValueTest, uint64BadCharInBw) {
    try {
        ArgValue av("1234x1234");
        av.value<uint64_t>();
    } catch (ArgTypeException& e) {
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST(ArgValueTest, uint64Overflow) {
    try {
        ArgValue av("18446744073709551616");
        av.value<uint64_t>();
    } catch (ArgTypeException& e) {
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST(ArgValueTest, uint64Empty) {
    try {
        ArgValue av("");
        av.value<uint64_t>();
    } catch (ArgTypeException& e) {
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST(ArgValueTest, uint32) {
    ArgValue av("1234");
    ASSERT_EQ(1234u, av.value<uint32_t>());
    av = ArgValue("4294967295");
    ASSERT_EQ(4294967295u, av.value<uint32_t>());
}

TEST(ArgValueTest, uint32BadCharBefore) {
    try {
        ArgValue av("x1234");
        av.value<uint32_t>();
    } catch (ArgTypeException& e) {
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST(ArgValueTest, uint32BadCharAfter) {
    try {
        ArgValue av("1234x");
        av.value<uint32_t>();
    } catch (ArgTypeException& e) {
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST(ArgValueTest, uint32BadCharInBw) {
    try {
        ArgValue av("1234x1234");
        av.value<uint32_t>();
    } catch (ArgTypeException& e) {
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST(ArgValueTest, uint32Overflow) {
    try {
        ArgValue av("4294967296");
        av.value<uint32_t>();
    } catch (ArgTypeException& e) {
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST(ArgValueTest, uint32Empty) {
    try {
        ArgValue av("");
        av.value<uint32_t>();
    } catch (ArgTypeException& e) {
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST(ArgValueTest, int64) {
    ArgValue av("1234");
    ASSERT_EQ(1234LL, av.value<int64_t>());
    av = ArgValue("-1234");
    ASSERT_EQ(-1234LL, av.value<int64_t>());
    av = ArgValue("9223372036854775807");
    ASSERT_EQ(9223372036854775807LL, av.value<int64_t>());
    av = ArgValue("-9223372036854775808");
    ASSERT_EQ(-9223372036854775807LL - 1, av.value<int64_t>());
    // Correct (indirect) way to write -2^63.
    // See http://en.cppreference.com/w/cpp/language/integer_literal
}

TEST(ArgValueTest, int64BadCharBefore) {
    try {
        ArgValue av("x1234");
        av.value<int64_t>();
    } catch (ArgTypeException& e) {
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST(ArgValueTest, int64BadCharAfter) {
    try {
        ArgValue av("1234x");
        av.value<int64_t>();
    } catch (ArgTypeException& e) {
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST(ArgValueTest, int64BadCharInBw) {
    try {
        ArgValue av("1234x1234");
        av.value<int64_t>();
    } catch (ArgTypeException& e) {
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST(ArgValueTest, int64Overflow) {
    try {
        ArgValue av("9223372036854775808");
        av.value<int64_t>();
    } catch (ArgTypeException& e) {
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST(ArgValueTest, int64Underflow) {
    try {
        ArgValue av("-9223372036854775809");
        av.value<int64_t>();
    } catch (ArgTypeException& e) {
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST(ArgValueTest, int64Empty) {
    try {
        ArgValue av("");
        av.value<int64_t>();
    } catch (ArgTypeException& e) {
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST(ArgValueTest, int32) {
    ArgValue av("1234");
    ASSERT_EQ(1234, av.value<int32_t>());
    av = ArgValue("-1234");
    ASSERT_EQ(-1234, av.value<int32_t>());
    av = ArgValue("2147483647");
    ASSERT_EQ(2147483647, av.value<int32_t>());
    av = ArgValue("-2147483648");
    ASSERT_EQ(-2147483648, av.value<int32_t>());
    // Correct (indirect) way to write -2^63.
    // See http://en.cppreference.com/w/cpp/language/integer_literal
}

TEST(ArgValueTest, int32BadCharBefore) {
    try {
        ArgValue av("x1234");
        av.value<int32_t>();
    } catch (ArgTypeException& e) {
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST(ArgValueTest, int32BadCharAfter) {
    try {
        ArgValue av("1234x");
        av.value<int32_t>();
    } catch (ArgTypeException& e) {
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST(ArgValueTest, int32BadCharInBw) {
    try {
        ArgValue av("1234x1234");
        av.value<int32_t>();
    } catch (ArgTypeException& e) {
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST(ArgValueTest, int32Overflow) {
    try {
        ArgValue av("2147483648");
        av.value<int32_t>();
    } catch (ArgTypeException& e) {
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST(ArgValueTest, int32Underflow) {
    try {
        ArgValue av("-2147483649");
        av.value<int32_t>();
    } catch (ArgTypeException& e) {
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST(ArgValueTest, int32Empty) {
    try {
        ArgValue av("");
        av.value<int32_t>();
    } catch (ArgTypeException& e) {
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST(ArgValueTest, float) {
    ArgValue av("1234");
    ASSERT_EQ_WithError<float>(1234, av.value<float>(), 1e-6);
    av = ArgValue("-1234");
    ASSERT_EQ_WithError<float>(-1234, av.value<float>(), 1e-6);
    av = ArgValue("12.34");
    ASSERT_EQ_WithError<float>(12.34, av.value<float>(), 1e-6);
    av = ArgValue("-12.34");
    ASSERT_EQ_WithError<float>(-12.34, av.value<float>(), 1e-6);
    av = ArgValue("12e34");
    ASSERT_EQ_WithError<float>(12e34, av.value<float>(), 1e-6);
    av = ArgValue("-12E+34");
    ASSERT_EQ_WithError<float>(-12e34, av.value<float>(), 1e-6);
    av = ArgValue("12e-34");
    ASSERT_EQ_WithError<float>(12e-34, av.value<float>(), 1e-6);
    av = ArgValue("infinity");
    ASSERT_EQ(std::numeric_limits<float>::infinity(), av.value<float>());
    av = ArgValue("-INF");
    ASSERT_EQ(-std::numeric_limits<float>::infinity(), av.value<float>());
    av = ArgValue("NAN");
    ASSERT_TRUE(std::isnan(av.value<float>()));
    av = ArgValue("-nan");
    ASSERT_TRUE(std::isnan(av.value<float>()));
}

TEST(ArgValueTest, floatBadCharBefore) {
    try {
        ArgValue av("x123.4");
        av.value<float>();
    } catch (ArgTypeException& e) {
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST(ArgValueTest, floatBadCharAfter) {
    try {
        ArgValue av("123e4x");
        av.value<float>();
    } catch (ArgTypeException& e) {
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST(ArgValueTest, floatBadCharInBw) {
    try {
        ArgValue av("1234ex12");
        av.value<float>();
    } catch (ArgTypeException& e) {
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST(ArgValueTest, floatOverflow) {
    try {
        ArgValue av("1.18e39");
        av.value<float>();
    } catch (ArgTypeException& e) {
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST(ArgValueTest, floatUnderflow) {
    try {
        ArgValue av("-1.18e39");
        av.value<float>();
    } catch (ArgTypeException& e) {
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST(ArgValueTest, floatEmpty) {
    try {
        ArgValue av("");
        av.value<float>();
    } catch (ArgTypeException& e) {
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST(ArgValueTest, double) {
    ArgValue av("1234");
    ASSERT_EQ_WithError<double>(1234, av.value<double>(), 1e-6);
    av = ArgValue("-1234");
    ASSERT_EQ_WithError<double>(-1234, av.value<double>(), 1e-6);
    av = ArgValue("12.34");
    ASSERT_EQ_WithError<double>(12.34, av.value<double>(), 1e-6);
    av = ArgValue("-12.34");
    ASSERT_EQ_WithError<double>(-12.34, av.value<double>(), 1e-6);
    av = ArgValue("12e34");
    ASSERT_EQ_WithError<double>(12e34, av.value<double>(), 1e-6);
    av = ArgValue("-12E+34");
    ASSERT_EQ_WithError<double>(-12e34, av.value<double>(), 1e-6);
    av = ArgValue("12e-34");
    ASSERT_EQ_WithError<double>(12e-34, av.value<double>(), 1e-6);
    av = ArgValue("infinity");
    ASSERT_EQ(std::numeric_limits<double>::infinity(), av.value<double>());
    av = ArgValue("-INF");
    ASSERT_EQ(-std::numeric_limits<double>::infinity(), av.value<double>());
    av = ArgValue("NAN");
    ASSERT_TRUE(std::isnan(av.value<double>()));
    av = ArgValue("-nan");
    ASSERT_TRUE(std::isnan(av.value<double>()));
}

TEST(ArgValueTest, doubleBadCharBefore) {
    try {
        ArgValue av("x123.4");
        av.value<double>();
    } catch (ArgTypeException& e) {
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST(ArgValueTest, doubleBadCharAfter) {
    try {
        ArgValue av("123e4x");
        av.value<double>();
    } catch (ArgTypeException& e) {
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST(ArgValueTest, doubleBadCharInBw) {
    try {
        ArgValue av("1234ex12");
        av.value<double>();
    } catch (ArgTypeException& e) {
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST(ArgValueTest, doubleOverflow) {
    try {
        ArgValue av("1.18e309");
        av.value<double>();
    } catch (ArgTypeException& e) {
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST(ArgValueTest, doubleUnderflow) {
    try {
        ArgValue av("-1.18e309");
        av.value<double>();
    } catch (ArgTypeException& e) {
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

TEST(ArgValueTest, doubleEmpty) {
    try {
        ArgValue av("");
        av.value<double>();
    } catch (ArgTypeException& e) {
        return;
    }

    // Never reach.
    ASSERT_TRUE(false);
}

