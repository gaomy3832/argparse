#ifndef ARGUMENT_PARSER_H_
#define ARGUMENT_PARSER_H_
/**
 * A simple command line argument/option parser.
 */
#include <functional>
#include <string>

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

} // namespace argparse

#endif
