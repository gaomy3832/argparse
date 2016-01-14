#ifndef ARGUMENT_PARSER_H_
#define ARGUMENT_PARSER_H_
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
    explicit ArgKeyException(const std::string& key) : key_(key) {}
    virtual ~ArgKeyException() {}
    const char* what() const throw() { return key_.c_str(); }
protected:
    std::string key_;
};

class ArgValueException : public std::exception {
public:
    ArgValueException(const std::string& val, const std::string& arg)
        : val_(val), arg_(arg)
    {
        // Nothing else to do.
    }
    virtual ~ArgValueException() {}
    const char* what() const throw() {
        auto str = val_ + ":->:" + arg_;
        return str.c_str();
    }
protected:
    std::string val_;
    std::string arg_;
};

class ArgTypeException : public std::exception {
public:
    ArgTypeException(const std::string& val, const std::string& type)
        : val_(val), type_(type)
    {
        // Nothing else to do.
    }
    virtual ~ArgTypeException() {}
    const char* what() const throw() {
        auto str = val_ + ":->:" + type_;
        return str.c_str();
    }
protected:
    std::string val_;
    std::string type_;
};



/************************
 * Generic argument type.
 ************************/

class ArgValue {
public:
    ArgValue(const std::string& str)
        : str_(str)
    {
        // Nothing else to do.
    }

    template<typename T>
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
    inline T safeStrTo(std::function<T(const char*, char**)> strto, const char* typeName) const {
        char* end = nullptr; errno = 0;
        auto v = strto(str_.c_str(), &end);
        if (str_.empty() || *end != '\0' || errno == ERANGE) {
            throw ArgTypeException(str_, typeName);
        }
        return v;
    }
};


template<> std::string ArgValue::value() const {
    return str_;
}

template<> uint64_t ArgValue::value() const {
    return safeStrTo<uint64_t>(std::bind(strtoull, std::placeholders::_1, std::placeholders::_2, 10), "uint64");
}

template<> uint32_t ArgValue::value() const {
    return safeStrTo<uint32_t>(std::bind(strtoul, std::placeholders::_1, std::placeholders::_2, 10), "uint32");
}

template<> int64_t ArgValue::value() const {
    return safeStrTo<int64_t>(std::bind(strtoll, std::placeholders::_1, std::placeholders::_2, 10), "int64");
}

template<> int32_t ArgValue::value() const {
    return safeStrTo<int32_t>(std::bind(strtol, std::placeholders::_1, std::placeholders::_2, 10), "int32");
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
    /**
     * Argument, including both the properties and the argument values.
     */
    class Argument {
    public:
        Argument(const std::string& name, const bool required, const std::string& defaultValue,
                const std::vector<std::string>& choices, const size_t expectCount, const std::string& help)
            : name_(name), required_(required), defaultValue_(defaultValue),
              choices_(choices.begin(), choices.end()), expectCount_(expectCount), help_(help)
        {
            if (!isChoice(defaultValue_)) {
                throw ArgValueException(defaultValue_, name_);
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
         * Argument value in the command line that is associated with this option.
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
        : positionalArgList_(), optionMap_()
    {
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

    template<typename T, typename KeyT>
    const T argValue(const KeyT& key, const size_t valueIdx = 0) const {
        auto arg = argument(key);
        if (arg == nullptr) throw ArgKeyException(strKey(key));
        return arg->argValue(valueIdx).template value<T>();
    }

    /* Positional argument. */

    size_t positionalArgCount() const {
        return positionalArgList_.size();
    }

    /* Option/flag. */

    bool optionGiven(const std::string& key) const {
        auto arg = argument(key);
        if (arg == nullptr) throw ArgKeyException(strKey(key));
        return arg->given();
    }


    template<typename T>
    void argumentNew(const std::string& name, const bool required, const T& defaultValue,
            const std::vector<T>& choices, const std::string& help,
            const size_t expectCount, const std::initializer_list<std::string>& aliases);

    void cmdlineIs(int argc, char* argv[]);

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

    static std::string strKey(const size_t& idx) { return "@" + std::to_string(idx); }

    static bool isFlag(const std::string& key) {
        return key.compare(0, 1, "-") == 0 || key.compare(0, 2, "--") == 0;
    }

};


template<typename T>
void ArgumentParser::argumentNew(const std::string& name, const bool required, const T& defaultValue,
        const std::vector<T>& choices, const std::string& help,
        const size_t expectCount, const std::initializer_list<std::string>& aliases) {

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
                optionMap_[a] = ptr;
            }
        } else {
            positionalArgList_.push_back(ptr);
        }
    } catch (ArgValueException& e) {
        std::cerr << "Invalid argument value: " << e.what() << std::endl;
        std::cerr << this->help() << std::endl;
        throw;
    }
}

void ArgumentParser::cmdlineIs(int argc, char* argv[]) {
    reset();
    if (argc <= 1) return;
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
                if (arg == nullptr) throw ArgKeyException(strKey(key));
                argi++;
            } else {
                arg = argument(posArgIdx);
                if (arg == nullptr) throw ArgKeyException(strKey(posArgIdx));
                posArgIdx++;
            }

            for (size_t i = 0; i < arg->expectCount(); i++, argi++) {
                if (argi >= argc || isFlag(argv[argi])) break;
                if (!arg->isChoice(argv[argi])) {
                    throw ArgValueException(std::string(argv[argi]), arg->name());
                }
                arg->argValueNew(argv[argi]);
            }
            arg->givenIs(true);
        }
    } catch (ArgKeyException& e) {
        std::cerr << "Unrecognized option or too many positional arguments: " << e.what() << std::endl;
        std::cerr << help() << std::endl;
        throw;
    } catch (ArgValueException& e) {
        std::cerr << "Invalid argument value: " << e.what() << std::endl;
        std::cerr << help() << std::endl;
        throw;
    }

    //TODO: do check.
}

} // namespace argparse

#endif
