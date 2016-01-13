#ifndef ARGUMENT_PARSER_H_
#define ARGUMENT_PARSER_H_
/**
 * A simple command line argument/option parser.
 */
#include <functional>
#include <string>
#include <unordered_set>
#include <vector>

namespace argparse {


/***********************
 * Exception definition.
 ***********************/

class ArgTypeException : public std::exception {
public:
    explicit ArgTypeException(const std::string& str) : str_(str) {}
    virtual ~ArgTypeException() {}
    const char* what() const throw() { return str_.c_str(); }
protected:
    std::string str_;
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
        if (str_.empty() || *end != '\0' || errno == ERANGE) throw ArgTypeException(typeName);
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
            // Nothing else to do.
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

        const size_t argValueCount() const {
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

};

} // namespace argparse

#endif
