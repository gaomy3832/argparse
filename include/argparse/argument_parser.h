/*
 * Copyright 2016 Mingyu Gao
 *
 */
#ifndef ARGPARSE_ARGUMENT_PARSER_H_
#define ARGPARSE_ARGUMENT_PARSER_H_
/**
 * @file
 *
 * @brief
 * A simple command line argument/option parser.
 */

#include <algorithm>
#include <cctype>
#include <functional>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace argparse {

/**
 * @defgroup exceptions
 *
 * @brief
 * Exception types.
 */
/**@{*/

/**
 * \brief Argument key exception.
 *
 * Used when the argument name is incorrect.
 */
class ArgKeyException : public std::exception {
public:
    explicit ArgKeyException(const std::string& key,
            const std::string& reason = "")
            : key_(key), str_(key_ + (reason.empty() ? "" : ": "+reason)) {
        // Nothing else to do.
    }
    virtual ~ArgKeyException() {}
    const char* key() const throw() { return key_.c_str(); }
    const char* what() const throw() { return str_.c_str(); }
protected:
    const std::string key_;
    const std::string str_;
};

/**
 * \brief Argument value exception.
 *
 * Used when the value of the argument is invalid.
 */
class ArgValueException : public std::exception {
public:
    explicit ArgValueException(const std::string& val,
            const std::string& reason = "")
            : val_(val), str_(val_ + (reason.empty() ? "" : ": "+reason)) {
        // Nothing else to do.
    }
    virtual ~ArgValueException() {}
    const char* val() const throw() { return val_.c_str(); }
    const char* what() const throw() { return str_.c_str(); }
protected:
    const std::string val_;
    const std::string str_;
};

/**
 * \brief Argument property exception.
 *
 * Used when the property of the argument is not correctly specified, or not
 * satisfied when parsing.
 */
class ArgPropertyException : public std::exception {
public:
    ArgPropertyException(const std::string& key, const std::string& property,
            const std::string& reason = "")
            : key_(key), property_(property),
              str_(key_+"."+property_ + (reason.empty() ? "" : ": "+reason)) {
        // Nothing else to do.
    }
    virtual ~ArgPropertyException() {}
    const char* key() const throw() { return key_.c_str(); }
    const char* property() const throw() { return property_.c_str(); }
    const char* what() const throw() { return str_.c_str(); }
protected:
    const std::string key_;
    const std::string property_;
    const std::string str_;
};

/**@}*/


/**
 * \brief Generic argument value type.
 *
 * String is used as the \c Any type. Convert to desired type lazily (i.e.,
 * raise error at the use cite).
 */
class ArgValue {
public:
    /**
     * \brief Construct argument value using string.
     */
    explicit ArgValue(const std::string& str)
            : str_(str) {
        // Nothing else to do.
    }

    /**
     * \brief Safe conversion argument value to type \c T.
     *
     * Throw \c ArgValueException if conversion fails.
     */
    template<typename T = std::string>
    T value() const;

private:
    // Use string as the \c Any type.
    std::string str_;

private:
    /**
     * \brief Safely convert string to type \c T.
     *
     * The whole string must be successfully converted. Also detect range
     * exceptions.
     *
     * \c sto* (\c stoull, etc.) silently succeeds when only a prefix is
     * converted.  Use \c strto*. See http://stackoverflow.com/a/6154614.
     */
    template<typename T>
    inline T safeStrTo(std::function<T(const char*, char**)> strto,
            const char* typeName) const {
        char* end = nullptr; errno = 0;
        auto v = strto(str_.c_str(), &end);
        if (str_.empty() || *end != '\0' || errno == ERANGE) {
            throwConvertException(typeName);
        }
        return v;
    }

    void throwConvertException(const std::string& typeName) const {
        throw ArgValueException(str_, "convert to " + typeName);
    }
};


template<> std::string ArgValue::value() const {
    return str_;
}

template<> uint64_t ArgValue::value() const {
    return safeStrTo<uint64_t>(std::bind(strtoull,
                std::placeholders::_1, std::placeholders::_2, 10), "uint64");
}

template<> uint32_t ArgValue::value() const {
    auto v = safeStrTo<uint64_t>(std::bind(strtoull,
                std::placeholders::_1, std::placeholders::_2, 10), "uint32");
    uint64_t mask = ~((1uLL << (8u*sizeof(uint32_t))) - 1);  // fff..f000..0
    if (v & mask) throwConvertException("uint32_t");
    return static_cast<uint32_t>(v);
}

template<> int64_t ArgValue::value() const {
    return safeStrTo<int64_t>(std::bind(strtoll,
                std::placeholders::_1, std::placeholders::_2, 10), "int64");
}

template<> int32_t ArgValue::value() const {
    auto v = safeStrTo<int64_t>(std::bind(strtol,
                std::placeholders::_1, std::placeholders::_2, 10), "int32");
    uint64_t mask = ~((1uLL << (8u*sizeof(uint32_t)-1)) - 1);  // fff..f800..0
    if ((v & mask) && ((~v) & mask)) throwConvertException("int32");
    return static_cast<int32_t>(v);
}

template<> float ArgValue::value() const {
    return safeStrTo<float>(strtof, "float");
}

template<> double ArgValue::value() const {
    return safeStrTo<double>(strtod, "double");
}



/**
 * \brief Argument parser.
 */
class ArgumentParser {
private:
    /**
     * \brief Argument, including both the properties and the argument values.
     */
    class Argument {
    public:
        /**
         * \brief Construct an argument.
         */
        Argument(const std::string& name, const std::string& help,
                const size_t expectCount, const bool required,
                const std::string& defaultValue,
                const std::vector<std::string>& choices)
                : name_(name), help_(help), expectCount_(expectCount),
                  required_(required), defaultValue_(defaultValue),
                  choices_(choices.begin(), choices.end()),
                  given_(false), argValueList_() {
            if (!isFlag(name_) && (expectCount_ == 0 || expectCount_ == -1uL)) {
                throw ArgPropertyException(name_, "expectCount",
                        "positional argument should not be 0 "
                        "or variable length");
            }
            if (isFlag(name_) && required && expectCount_ == 0) {
                throw ArgPropertyException(name_, "required",
                        "pure flag should not be required");
            }
            if (!isChoice(defaultValue_)) {
                throw ArgPropertyException(name_, "defaultValue",
                        "default value is not a choice for " + name_);
            }
        }

        std::string name() const { return name_; }
        bool required() const { return required_; }
        std::string defaultValue() const { return defaultValue_; }
        size_t expectCount() const { return expectCount_; }
        std::string help() const { return help_; }

        /**
         * \brief Whether the \c value is a valid choice.
         */
        bool isChoice(const std::string& value) const {
            return choices_.empty() || choices_.count(value) != 0;
        }

        /**
         * \brief Whether this argument is given at parse time.
         */
        bool given() const { return given_; }
        void givenIs(bool given) { given_ = given; }

        /**
         * \brief Get the <tt>idx</tt>-th parsed argument value.
         */
        const ArgValue argValue(const size_t idx) const {
            if (idx >= argValueList_.size()) {
                return ArgValue("");
            }
            return argValueList_[idx];
        }

        /**
         * \brief Number of parsed argument values.
         */
        size_t argValueCount() const {
            return argValueList_.size();
        }

        /**
         * \brief Add an parsed argument value \c arg.
         */
        void argValueNew(const std::string& arg) {
            argValueList_.push_back(ArgValue(arg));
        }

        /**
         * \brief Remove all parsed argument values.
         */
        void argValueDelAll() {
            argValueList_.clear();
            given_ = false;
        }

    private:
        // Properties.
        std::string name_;
        std::string help_;
        size_t expectCount_;
        bool required_;
        std::string defaultValue_;
        std::unordered_set<std::string> choices_;

        // Argument values.
        bool given_;
        std::vector<ArgValue> argValueList_;
    };

    static constexpr size_t indent = 4;
    static constexpr size_t maxLinewidth = 80;

public:
    /**
     * \brief Initialize ArgumentParser.
     */
    explicit ArgumentParser(const std::string description)
            : description_(description),
              positionalArgList_(), optionMap_(), aliasMap_() {
        // Nothing else to do.
    }

    /**
     * \brief Get the usage (short help) message.
     */
    const std::string usage(const std::string& binName) const;

    /**
     * \brief Get the help message.
     */
    const std::string help(const std::string& binName = "") const;

    /**
     * @name Generic argument value access.
     */
    /**@{*/

    /**
     * \brief Get number of parsed argument values.
     *
     * @param key  the argument name. An integer for positional argument,
     * and a flag string for option.
     *
     * @return  the number parsed argument values.
     */
    template<typename KeyT>
    size_t argValueCount(const KeyT& key) const {
        auto arg = argument(key);
        if (arg == nullptr) return 0;
        return arg->argValueCount();
    }

    /**
     * \brief Get the <tt>valueIdx</tt>-th parsed argument value for the
     * argument with name \c key.
     *
     * @param key  the argument name. An integer for positional argument,
     * and a flag string for option.
     * @param valueIdx  the index to the value.
     *
     * @return  the argument value or an empty string if \c valueIdx exceeds
     * the number of values.
     */
    template<typename T = std::string, typename KeyT>
    const T argValue(const KeyT& key, const size_t valueIdx = 0) const {
        auto arg = argument(key);
        if (arg == nullptr)
            throw ArgKeyException(strKey(key), "invalid argument name");
        return arg->argValue(valueIdx).template value<T>();
    }

    /**@}*/

    /**
     * @name Positional argument.
     */
    /**@{*/

    /**
     * \brief Number of positional arguments.
     */
    size_t positionalArgCount() const {
        return positionalArgList_.size();
    }

    /**@}*/

    /**
     * @name Option/flag.
     */
    /**@{*/

    /**
     * \brief Check if an option is given.
     *
     * @param key  a flag string for the option.
     *
     * @return  whether the option is given.
     */
    bool optionGiven(const std::string& key) const {
        auto arg = argument(key);
        if (arg == nullptr)
            throw ArgKeyException(strKey(key), "invalid option name");
        return arg->given();
    }

    /**@}*/

    /**
     * \brief Add an argument.
     *
     * @param name  if starting with \c - or \c -- and following by an
     * alphabetic character, it is an option; otherwise it is a positional
     * argument.
     *
     * @param help  help message.
     *
     * @param expectCount  for positional argument, it should be positive
     * integer; for option, 0 means pure flag, -1 means any number
     * (including 0), i.e., *.
     *
     * @param required  if true, must give \c expectCount arguments;
     * otherwise use \c defaultValue to fill in. In case of <tt>expectCount
     * == -1</tt>, \c required is true means at least one number needs to be
     * provide. No \c defaultValue will be filled in in case of <tt>required
     * == false</tt>.
     *
     * @param defaultValue  default value if none is given.
     *
     * @param choices  given value and \c defaultValue must be in \c
     * choices. Empty means all values.
     *
     * @param aliases  aliases for the option. Must also be flags.
     */
    template<typename T>
    void argumentNew(const std::string& name, const std::string& help,
            const size_t expectCount, const bool required = true,
            const T& defaultValue = T{}, const std::vector<T>& choices = {},
            const std::initializer_list<std::string>& aliases = {});

    /**
     * \brief Parse a command line.
     *
     * @param argc, argv  the command line.
     */
    /**@{*/

    void cmdlineIs(int argc, const char* argv[]);

    void cmdlineIs(int argc, char* argv[]);

    /**@}*/

private:
    // Description.
    const std::string description_;

    // Positional argument list.
    std::vector<std::shared_ptr<Argument>> positionalArgList_;
    // Option name to argument.
    std::unordered_map<std::string, std::shared_ptr<Argument>> optionMap_;

    // Alias to option name.
    std::unordered_map<std::string, std::string> aliasMap_;

private:
    void reset() {
        for (auto& pa : positionalArgList_) {
            pa->argValueDelAll();
        }
        for (auto& kv : optionMap_) {
            if (aliasMap_.count(kv.first) == 0) {
                kv.second->argValueDelAll();
            }
        }
    }

    void checkArgument(const std::shared_ptr<Argument>& arg) {
        if (arg->required()) {
            // Expected value count must be given for required argument.
            if (!arg->given()) {
                throw ArgPropertyException(arg->name(), "required",
                        "required but not given");
            }
            if (arg->expectCount() != -1uL
                    && arg->argValueCount() != arg->expectCount()) {
                throw ArgPropertyException(arg->name(), "expectCount",
                        "too few arguments");
            }
        } else {
            if (arg->expectCount() != -1uL) {
                // If not required, use default values to fill in the blank.
                for (auto idx = arg->argValueCount(); idx < arg->expectCount();
                        idx++) {
                    arg->argValueNew(arg->defaultValue());
                }
            }
        }
    }

    std::shared_ptr<Argument> argument(const std::string& key) const {
        auto iter = optionMap_.find(key);
        if (iter == optionMap_.end()) {
            return nullptr;
        }
        return iter->second;
    }

    std::shared_ptr<Argument> argument(const size_t& idx) const {
        if (idx >= positionalArgList_.size()) {
            return nullptr;
        }
        return positionalArgList_[idx];
    }

    static std::string strKey(const std::string& key) { return key; }

    static std::string strKey(const size_t& idx) {
        return "@" + std::to_string(idx);
    }

    static bool isFlag(const std::string& key) {
        // Flag starts with - or -- and follows by an alphabetic character.
        return (key.compare(0, 1, "-") == 0 && isalpha(key[1]))
                || (key.compare(0, 2, "--") == 0 && isalpha(key[2]));
    }

    static std::string breakLines(const std::string str, const size_t lineLen,
            const size_t indent) {
        std::string fmtStr;
        auto textLen = lineLen - indent;

        std::string::size_type pos = 0;
        while (pos < str.size()) {
            // Add indent.
            fmtStr += std::string(indent, ' ');

            // Find break.
            // First find explicit newline.
            auto llen = str.substr(pos, textLen).find_first_of("\n");
            if (llen == std::string::npos
                    && str.size() - pos > textLen) {
                // If no newline but still longer than a line,
                // break at the last whitespace.
                llen = str.substr(pos, textLen).find_last_of(" ");
                if (llen == std::string::npos) {
                    // This should be really rare: no whitespace!
                    llen = str.find_first_of(" ");
                }
            }

            fmtStr += str.substr(pos, llen);
            fmtStr += "\n";
            if (llen == std::string::npos) break;

            // Skip all whitespaces.
            pos = str.find_first_not_of("\n ", pos + llen);
        }
        return fmtStr;
    }
};


const std::string ArgumentParser::usage(const std::string& binName) const {
    std::string str;

    str += binName;
    str += " ";

    // Print the meta variables.
    auto getMetaVars = [this](const std::shared_ptr<Argument>& arg) {
        std::string str;

        // Decide the metavar.
        std::string metavar = "X";
        if (isFlag(arg->name())) {
            auto pos = arg->name().find_first_not_of("-");
            if (pos != std::string::npos) {
                metavar = std::string(1, std::toupper(arg->name()[pos]));
            }
        } else {
            metavar = arg->name();
        }

        // Print metavars according to \c expectCount.
        if (arg->expectCount() == -1uL) {
            str += " " + metavar + " ...";
        } else {
            for (size_t i = 0; i < arg->expectCount(); i++) {
                str += ((!isFlag(arg->name()) && i == 0) ? "" : " ") + metavar;
            }
        }

        return str;
    };

    for (auto& kv : optionMap_) {
        if (aliasMap_.count(kv.first) == 0) {
            auto option = kv.second;
            if (!option->required()) str += "[";
            str += option->name();
            str += getMetaVars(option);
            if (!option->required()) str += "]";
            str += " ";
        }
    }
    for (auto& pa : positionalArgList_) {
        if (!pa->required()) str += "[";
        str += getMetaVars(pa);
        if (!pa->required()) str += "]";
        str += " ";
    }
    str += "\n";

    // Format.
    str = breakLines(str, maxLinewidth, indent);

    str = description_ + "\n\n" + "Usage:\n" + str + "\n";

    return str;
}

const std::string ArgumentParser::help(const std::string& binName) const {
    const size_t maxIndent2 = maxLinewidth / 2;

    // Get length of longest name.
    size_t maxLength = 0;
    for (auto& pa : positionalArgList_) {
        maxLength = std::max(maxLength, pa->name().size());
    }
    for (auto& kv : optionMap_) {
        if (aliasMap_.count(kv.first) == 0) {
            maxLength = std::max(maxLength, kv.second->name().size());
        }
    }

    // Indent of the help column, should be multiple of \c indent.
    // \c indent before name column, name column is \c maxLength, and \c
    // indent after name column.
    size_t indent2 = (indent + maxLength + indent
            + indent - 1) / indent * indent;
    // If too large, shrink it.
    indent2 = std::min(maxIndent2, indent2);

    std::string str;

    if (binName.size() > 0) {
        str += usage(binName);
        str += "\n";
    }

    str += "Positional arguments:\n\n";
    for (auto& pa : positionalArgList_) {
        str += std::string(indent, ' ');
        str += pa->name();
        str += "\n";

        str += breakLines(pa->help(), maxLinewidth, indent2);
        str += "\n";
    }
    str += "\n";

    str += "Options:\n\n";
    for (auto& kv : optionMap_) {
        if (aliasMap_.count(kv.first) == 0) {
            str += std::string(indent, ' ');
            str += kv.second->name();
            std::vector<std::string> aliases;
            // Figure out aliases.
            for (auto& kv2 : aliasMap_) {
                if (kv.first == kv2.second) {
                    str += "," + kv2.first;
                }
            }
            str += "\n";

            str += breakLines(kv.second->help(), maxLinewidth, indent2);
            str += "\n";
        }
    }
    str += "\n";

    return str;
}

template<typename T>
void ArgumentParser::argumentNew(const std::string& name,
        const std::string& help,
        const size_t expectCount, const bool required,
        const T& defaultValue, const std::vector<T>& choices,
        const std::initializer_list<std::string>& aliases) {
    // Convert all types (including string and char[]) to string.
    // \c std::to_string only works for numeric types.
    auto allToString = [](const T& var) {
        std::stringstream ss;
        ss << var;
        return ss.str();
    };

    auto strDefaultValue = allToString(defaultValue);
    std::vector<std::string> strChoices;
    for (const auto& c : choices) {
        strChoices.push_back(allToString(c));
    }

    auto ptr = std::make_shared<Argument>(
            name, help, expectCount, required, strDefaultValue, strChoices);

    if (isFlag(name)) {
        optionMap_[name] = ptr;
        for (const auto& a : aliases) {
            if (!isFlag(a)) {
                throw ArgPropertyException(ptr->name(), "alias",
                        "alias for flag must also be a flag");
            }
            optionMap_[a] = ptr;
            aliasMap_[a] = name;
        }
    } else {
        if (!positionalArgList_.empty()
                && !positionalArgList_.back()->required()
                && ptr->required()) {
            throw ArgPropertyException(ptr->name(), "required",
                    "no required positional argument should be after "
                    "non-required ones");
        }
        positionalArgList_.push_back(ptr);
    }
}

void ArgumentParser::cmdlineIs(int argc, char* argv[]) {
    const char** cargv = new const char* [argc + 1];
    for (int idx = 0; idx < argc; idx++) {
        cargv[idx] = argv[idx];
    }
    cargv[argc] = nullptr;
    cmdlineIs(argc, cargv);
    delete cargv;
}

void ArgumentParser::cmdlineIs(int argc, const char* argv[]) {
    reset();

    // Skip program name.
    argc -= 1;
    argv += 1;

    int argi = 0;
    size_t posArgIdx = 0;
    while (argi < argc) {
        std::shared_ptr<Argument> arg = nullptr;
        if (isFlag(argv[argi])) {
            std::string key = argv[argi];
            arg = argument(key);
            if (arg == nullptr)
                throw ArgKeyException(strKey(key),
                        "invalid option encountered");
            argi++;
        } else {
            arg = argument(posArgIdx);
            if (arg == nullptr)
                throw ArgKeyException(strKey(posArgIdx),
                        "too many positional arguments");
            posArgIdx++;
        }

        for (size_t i = 0; i < arg->expectCount() && argi < argc
                && !isFlag(argv[argi]); i++, argi++) {
            if (!arg->isChoice(argv[argi])) {
                throw ArgValueException(std::string(argv[argi]),
                        "given value is not a choice for " + arg->name());
            }
            arg->argValueNew(argv[argi]);
        }
        arg->givenIs(true);
    }

    // Check whether required arguments and expected value counts are given.
    for (auto& arg : positionalArgList_) {
        checkArgument(arg);
    }
    for (auto& kv : optionMap_) {
        if (aliasMap_.count(kv.first) == 0) {
            checkArgument(kv.second);
        }
    }
}

}  // namespace argparse

#endif  // ARGPARSE_ARGUMENT_PARSER_H_
