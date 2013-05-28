#pragma once
#ifndef _SBL_OPTIONS_H
#define _SBL_OPTIONS_H

#include <vector>
#include <ostream>

/// @file sbl_options.h

/// Stretch Base Library
namespace SBL {

class OptionSet;

/// Base class for all options
class OptionBase {
public:
    /// option name
    const char*  name() const { return _name; }
    /// option help message
    const char*  help() const { return _help; }
    /// if option was used on the command line
    bool         used() const { return _used; }
    /// virtual destructor for a polymorphic class
    virtual ~OptionBase() {}
protected:
    OptionBase(OptionSet* parent, const char* name, const char* help);
    void write_error(const char* format, ...);
private:
    OptionBase();   // unimplemented
    OptionSet*      _parent;
    const char*     _name;
    const char*     _help;
    bool            _used;

    friend class OptionSet;
    friend std::ostream& operator<<(std::ostream& str, const OptionSet& option_set);
    virtual void set_from_string(const char* value) = 0;
    virtual void write(std::ostream& str) = 0;
    virtual void deflt(std::ostream& str) = 0;
};

/// individual option
template<typename T>
class Option : public OptionBase {
public:
    /// constructor, with parent, name, default value and help message
    Option(OptionSet* parent, const char* name, T deflt, const char* help) :
        OptionBase(parent, name, help), _value(deflt), _deflt(deflt) {}
    /// access the option value
    operator T() const { return _value; }
    /// write the option to the output stream
    void write(std::ostream& str) { str << name() << '=' << _value << std::endl; }
    void deflt(std::ostream& str) { str << _deflt; }
private :
    T   _value;
    T   _deflt;
    void set_from_string(const char* value);
};

/// all the options
class OptionSet {
public:
    typedef const char* String;
    /// header is printed before each option help, footer after
    OptionSet(const char* header = 0, const char* footer = 0);
    /// this *must* be called in the constructor, to parse arguments
    void        parse(int argc, char* argv[]);
    /// if help was requested, print it and return true
    bool        help(std::ostream& str);
    /// return error message or NULL if no errors
    const char* error() const { return _error[0] ? _error : NULL; }
    /// how many positional arguments we found
    size_t      pos_args_count() const { return _pos_args.size(); }
    /// positional argument i
    const char* pos_arg(unsigned int i) const { return i < _pos_args.size() ? _pos_args[i] : 0; }
private:
    typedef std::vector<OptionBase*> Options;
    Options                     _options;
    std::vector<const char*>    _pos_args;
    const char*                 _prog_name;
    const char*                 _header;
    const char*                 _footer;
    char                        _error[100];
    bool                        _help_req;
    static const int HELP_SPACE = 40;

    friend std::ostream& operator<<(std::ostream& str, const OptionSet& option_set);
    friend class OptionBase;

    OptionBase* find(const char* name);
    OptionBase* find_non_ambig(const char* name);
    static int match_len(const char* s1, const char* s2);
    void write_error(const char* format, ...);
    const char* type_name(OptionBase* option);
};

}
#endif
