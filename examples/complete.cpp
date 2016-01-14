#include <iostream>
#include "argument_parser.h"

using namespace argparse;

int main(int argc, char* argv[]) {
    ArgumentParser ap;

    ap.argumentNew(
            "a",
            true,
            "",
            {},
            "Positional argument 1.",
            1,
            {});

    ap.argumentNew(
            "b",
            true,
            100,
            {100, 200, 300},
            "Positional argument 2 with choices from 100, 200, 300.",
            1,
            {});

    ap.argumentNew(
            "c",
            false,
            10,
            {},
            "Positional argument 3, optional, and with default.",
            1,
            {});

    ap.argumentNew(
            "-h",
            false,
            "",
            {},
            "Print help message.",
            0,
            {"-help", "--help"});

    ap.argumentNew(
            "-f",
            false,
            "",
            {},
            "Simple flag with a very long help description that will be split"
            "automatically into a two column format when usage is printed for"
            "this program. Newlines will also help with justification.\nFor"
            "example:\n0 - an item\n1 - another item\n2 - and another item.",
            0,
            {"-flag", "--flag"});

    ap.argumentNew(
            "-l",
            false,
            "",
            {},
            "A list of arbitrary length.",
            -1,
            {"-list", "--list"});

    ap.argumentNew(
            "-s",
            true,
            "",
            {},
            "Single string which is required.",
            1,
            {"-str", "--str", "-string", "--string"});

    ap.argumentNew(
            "-i",
            false,
            1,
            {1,2,3,4},
            "Integer from 1, 2, 3, 4.",
            2,
            {});

    ap.argumentNew(
            "-u",
            false,
            10,
            {10u, 20u, 30u, 40u},
            "Unsigned integer from 10, 20, 30, 40.",
            1,
            {});

    ap.argumentNew(
            "-float",
            false,
            "",
            {},
            "Float number.",
            1,
            {});

    ap.argumentNew(
            "-double",
            false,
            "",
            {},
            "Double number.",
            1,
            {});

    ap.cmdlineIs(argc, argv);

    return 0;
}
