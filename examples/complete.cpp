#include <iostream>
#include "argparse/argument_parser.h"

using namespace argparse;

int main(int argc, const char* argv[]) {
    ArgumentParser ap;

    ap.argumentNew<std::string>(
            "a",
            "Positional argument 1.",
            1);

    ap.argumentNew<int>(
            "b",
            "Positional argument 2 with choices from 100, 200, 300.",
            1,
            true,
            100,
            {100, 200, 300});

    ap.argumentNew<int>(
            "c",
            "Positional argument 3, optional, and with default.",
            1,
            false,
            10);

    ap.argumentNew(
            "-h",
            "Print help message.",
            0,
            false,
            "",
            {},
            {"-help", "--help"});

    ap.argumentNew(
            "-f",
            "Simple flag with a very long help description that will be split"
            "automatically into a two column format when usage is printed for"
            "this program. Newlines will also help with justification.\nFor"
            "example:\n0 - an item\n1 - another item\n2 - and another item.",
            0,
            false,
            "",
            {},
            {"-flag", "--flag"});

    ap.argumentNew(
            "-l",
            "A list of arbitrary length.",
            -1,
            false,
            "",
            {},
            {"-list", "--list"});

    ap.argumentNew(
            "-s",
            "Single string which is required.",
            1,
            true,
            "",
            {},
            {"-str", "--str", "-string", "--string"});

    ap.argumentNew(
            "-i",
            "Integer from 1, 2, 3, 4.",
            2,
            false,
            1,
            {1,2,3,4});

    ap.argumentNew(
            "-u",
            "Unsigned integer from 10, 20, 30, 40.",
            1,
            false,
            10,
            {10u, 20u, 30u, 40u});

    ap.argumentNew<float>(
            "-float",
            "Float number.",
            1,
            false);

    ap.argumentNew<double>(
            "-double",
            "Double number.",
            1,
            false);

    ap.cmdlineIs(argc, argv);

    return 0;
}
