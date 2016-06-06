/*
 * Copyright 2016 Mingyu Gao
 *
 */
#ifndef ARGPARSE_ARGUMENT_PARSER_H_
#define ARGPARSE_ARGUMENT_PARSER_H_
/**
 * A simple command line argument/option parser.
 */
#include <functional>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace argparse {


/***********************
 * Exception definition.
 ***********************/

class ArgKeyException : public std::exception {
public:
    ArgKeyException(const std::string& key, const std::string& reason)
            : key_(key), reason_(reason),
              str_(key_ + ": " + reason_) {
        // Nothing else to do.
    }
    virtual ~ArgKeyException() {}
    const char* key() const throw() { return key_.c_str(); }
    const char* what() const throw() { return str_.c_str(); }
protected:
    std::string key_;
    std::string reason_;
    std::string str_;
};

class ArgValueException : public std::exception {
public:
    ArgValueException(const std::string& val, const std::string& reason)
            : val_(val), reason_(reason),
              str_(val_ + ": " + reason_) {
        // Nothing else to do.
    }
    virtual ~ArgValueException() {}
    const char* val() const throw() { return val_.c_str(); }
    const char* what() const throw() { return str_.c_str(); }
protected:
    std::string val_;
    std::string reason_;
    std::string str_;
};

class ArgPropertyException : public std::exception {
public:
    ArgPropertyException(const std::string& key, const std::string& property,
            const std::string& reason)
            : key_(key), property_(property), reason_(reason),
              str_(key_ + "." + property_ + ": " + reason_) {
        // Nothing else to do.
    }
    virtual ~ArgPropertyException() {}
    const char* key() const throw() { return key_.c_str(); }
    const char* property() const throw() { return property_.c_str(); }
    const char* what() const throw() { return str_.c_str(); }
protected:
    std::string key_;
    std::string property_;
    std::string reason_;
    std::string str_;
};



/************************
 * Generic argument type.
 ************************/

class ArgValue {
public:
    explicit ArgValue(const std::string& str)
            : str_(str) {
        // Nothing else to do.
    }

    template<typename T = std::string>
    T value() const;

protected:
    /**
     * Use string as the \c any type.
     */
    std::string str_;

protected:
    /**
     * Safely convert string to type T. The whole string must be successfully
     * converted. Also detect range exceptions.
     *
     * sto* (stoull, etc.) silently succeeds when only a prefix is converted.
     * Use strto*. See http://stackoverflow.com/a/6154614.
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



/******************
 * Argument parser.
 ******************/

class ArgumentParser {
protected:
    /** Argument, including both the properties and the argument values.
     *
     * \c name: if starting with \c - or \c --, it is an option; otherwise is a
     * positional argument.
     *
     * \c required: if true, must give \c expectCount arguments; otherwise use
     * \c defaultValue to fill in.
     *
     * \c defaultValue: default value if none is given.
     *
     * \c choices: given value and \c defaultValue must be in \c choices. Empty
     * means all values.
     *
     * \c expectCount: for positional argument, it cannot be 0 or -1; for
     * option, 0 means pure flag, -1 means any number (including 0), i.e., *.
     *
     * \c help: help message.
     */
    class Argument {
    public:
        Argument(const std::string& name, const bool required,
                const std::string& defaultValue,
                const std::vector<std::string>& choices,
                const size_t expectCount, const std::string& help)
                : name_(name), required_(required),
                  defaultValue_(defaultValue),
                  choices_(choices.begin(), choices.end()),
                  expectCount_(expectCount), help_(help) {
            if (!isFlag(name_) && (expectCount_ == 0 || expectCount_ == -1uL)) {
                throw ArgPropertyException(name_, "expectCount",
                        "positional argument should not be 0 "
                        "or variable length");
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

        bool isChoice(const std::string& value) const {
            return choices_.empty() || choices_.count(value) != 0;
        }

        /**
         * Whether this option is given in the command line.
         */
        bool given() const { return given_; }
        void givenIs(bool given) { given_ = given; }

        /**
         * Argument value in the command line that is associated with this
         * option.
         */
        const ArgValue argValue(const size_t idx) const {
            if (idx >= argValueList_.size()) {
                return ArgValue("");
            }
            return argValueList_[idx];
        }

        size_t argValueCount() const {
            return argValueList_.size();
        }

        void argValueNew(const std::string& arg) {
            argValueList_.push_back(ArgValue(arg));
        }

        void argValueDelAll() {
            argValueList_.clear();
        }

    protected:
        /* Properties. */
        std::string name_;
        bool required_;
        std::string defaultValue_;
        std::unordered_set<std::string> choices_;
        size_t expectCount_;
        std::string help_;

        /* Argument values. */
        bool given_;
        std::vector<ArgValue> argValueList_;
    };

public:
    ArgumentParser()
            : positionalArgList_(), optionMap_() {
        // Nothing else to do.
    }

    const std::string help() const {
        return "TODO\n";
    }

    /* Generic argument access. */

    template<typename KeyT>
    size_t argValueCount(const KeyT& key) const {
        auto arg = argument(key);
        if (arg == nullptr) return 0;
        return arg->argValueCount();
    }

    template<typename T = std::string, typename KeyT>
    const T argValue(const KeyT& key, const size_t valueIdx = 0) const {
        auto arg = argument(key);
        if (arg == nullptr)
            throw ArgKeyException(strKey(key), "invalid argument name");
        return arg->argValue(valueIdx).template value<T>();
    }

    /* Positional argument. */

    size_t positionalArgCount() const {
        return positionalArgList_.size();
    }

    /* Option/flag. */

    bool optionGiven(const std::string& key) const {
        auto arg = argument(key);
        if (arg == nullptr)
            throw ArgKeyException(strKey(key), "invalid option name");
        return arg->given();
    }


    template<typename T>
    void argumentNew(const std::string& name, const bool required,
            const T& defaultValue, const std::vector<T>& choices,
            const std::string& help, const size_t expectCount,
            const std::initializer_list<std::string>& aliases);

    void cmdlineIs(int argc, const char* argv[]);

protected:
    std::vector<std::shared_ptr<Argument>> positionalArgList_;
    std::unordered_map<std::string, std::shared_ptr<Argument>> optionMap_;

protected:
    void reset() {
        for (auto& pa : positionalArgList_) {
            pa->argValueDelAll();
        }
        for (auto& kv : optionMap_) {
            kv.second->argValueDelAll();
        }
    }

    void checkArgument(const std::shared_ptr<Argument>& arg) {
        if (arg->required()) {
            // Expected value count must be given for required argument.
            if (!arg->given() || arg->argValueCount() != arg->expectCount()) {
                throw ArgKeyException(arg->name(),
                        "required but not given, or too few arguments");
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
        return key.compare(0, 1, "-") == 0 || key.compare(0, 2, "--") == 0;
    }
};


template<typename T>
void ArgumentParser::argumentNew(const std::string& name, const bool required,
        const T& defaultValue, const std::vector<T>& choices,
        const std::string& help, const size_t expectCount,
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

    try {
        auto ptr = std::make_shared<Argument>(
                name, required, strDefaultValue, strChoices, expectCount, help);

        if (isFlag(name)) {
            optionMap_[name] = ptr;
            for (const auto& a : aliases) {
                if (!isFlag(a)) {
                    throw ArgPropertyException(ptr->name(), "alias",
                            "alias for flag must also be a flag");
                }
                optionMap_[a] = ptr;
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
    } catch (ArgPropertyException& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << this->help() << std::endl;
        throw;
    }
}

void ArgumentParser::cmdlineIs(int argc, const char* argv[]) {
    reset();

    // Skip program name.
    argc -= 1;
    argv += 1;

    try {
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

            for (size_t i = 0; i < arg->expectCount(); i++, argi++) {
                if (argi >= argc || isFlag(argv[argi])) break;
                if (!arg->isChoice(argv[argi])) {
                    throw ArgValueException(std::string(argv[argi]),
                            "given value is not a choice for " + arg->name());
                }
                arg->argValueNew(argv[argi]);
            }
            arg->givenIs(true);
        }
    } catch (ArgKeyException& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << help() << std::endl;
        throw;
    } catch (ArgValueException& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << help() << std::endl;
        throw;
    }

    // Check whether required arguments and expected value counts are given.
    try {
        for (auto& arg : positionalArgList_) {
            checkArgument(arg);
        }
        for (auto& kv : optionMap_) {
            checkArgument(kv.second);
        }
    } catch (ArgKeyException& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << help() << std::endl;
        throw;
    }
}

}  // namespace argparse

#endif  // ARGPARSE_ARGUMENT_PARSER_H_
