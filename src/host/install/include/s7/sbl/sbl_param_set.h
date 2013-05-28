#pragma once
#ifndef _SBL_PARAM_SET_H
#define _SBL_PARAM_SET_H
/****************************************************************************\
*  Copyright C 2012 Stretch, Inc. All rights reserved. Stretch products are  *
*  protected under numerous U.S. and foreign patents, maskwork rights,       *
*  copyrights and other intellectual property laws.                          *
*                                                                            *
*  This source code and the related tools, software code and documentation,  *
*  and your use thereof, are subject to and governed by the terms and        *
*  conditions of the applicable Stretch IDE or SDK and RDK License Agreement *
*  (either as agreed by you or found at www.stretchinc.com). By using these  *
*  items, you indicate your acceptance of such terms and conditions between  *
*  you and Stretch, Inc. In the event that you do not agree with such terms  *
*  and conditions, you may not use any of these items and must immediately   *
*  destroy any copies you have made.                                         *
\****************************************************************************/
#include <ostream>
#include <functional>
#include <map>
#include <set>
#include <string>
#include <boost/function.hpp>
#include "sbl_map.h"
#include "sbl_exception.h"

/*! @file sbl_param_set.h
    Declares ParamSet and Param<T> classes, which are useful for storing parameters.
    See detailed description of ParamSet for details.
*/

/// Stretch Base Library
namespace SBL {
class ParamSet;

/// Convert from type V to type T
template<typename T, typename V> 
T convert(V, bool abort = true);

/// Abstract base class for Param
/** Templated Param must derive from this class. Pointers to ParamBase are kept in ParamSet map */
class ParamBase {
public:
    /// Constructor must provide Param name and pointer to parent.
    /** Since ParamSet derives also from ParamBase, it is possible to have hierarchical structures: @verbatim
            struct FirstLevel : public ParamSet {
                Param<int> int;
                Param<bool> bool;
            };
            struct SecondLevel: public ParamSet {
                FirstLevel first_level;
            }; @endverbatim
    */
    ParamBase(ParamSet* parent, const char* name);
    /// Return Param name.
    const char* name() const { return _name.c_str(); }
    /// Reset current and backup to default
    virtual void reset() = 0;
    /// Revert current value from backup
    virtual void revert() = 0;
    /// Checkpoint, i.e. save current value in backup
    virtual void checkpt() = 0;
    /// true if value changed compared to last checkpoint
    virtual bool changed() const = 0;
    /// Virtual destructor because ParamBase is polymorphic
    virtual ~ParamBase() {}
    /// Friend stream operator for output
    friend std::ostream& operator<<(std::ostream& str, const ParamBase& param);
private:
    friend class ParamSet;
    /// copy value from another ParamBase
    virtual void copy_value(const ParamBase*) = 0;
    /// write value to a stream
    virtual void write(std::ostream& str) const = 0;
    /// Set value for the concrete type, reading from string
    virtual void set_from_string(const char* value, bool abort = true) = 0;
    /// initialize from string
    virtual void init(const char* value, ParamSet* parent, int index) = 0;
    /// append [%d] index to the name
    void add_index(int n);

    ParamBase();                 // private and unimplemented default constructor
    ParamBase(const ParamBase&); // private and unimplemented copy constructor
    ParamBase& operator=(const ParamBase&); // private and unimplemented assignment operator
    std::string     _name;
};    

/// Base class for parameter sets.
/** The following example illustrates how it can be used: @verbatim
    class Image : public ParamSet {
    public:
        Param<bool> osd;
        Param<int>  hue;
        Param<string> encoder;
        Image() : ParamSet("image"), 
                  osd(this, "osd", false), 
                  hue(this, "hue", 127, VerifyRange(0, 255)), 
                  encoder(this, "encoder", "MJPEG", VerifyEnum("MJPEG", "H264")) {}
    }; @endverbatim
    With this declaration, you can do the following: @verbatim
    Image image;
    image.osd = true;           // accessing member directly
    image.set("hue", 130);      // accessing member by name
    image.encoder = "MJPEG";    // accessing memer directly
    cout << image;              // printing structure to cout in name=value format
    ParamSet::ArgMap arg_map;   // create argument map
    arg_map["hue"] = "false";   // note that string is assigned
    arg_map["osd"] = "true";
    image.set(arg_map);         // set all parameters specified in the map
    @endverbatim
    @attention ParamSet throws Exception if the member does not exists when accessed by name (set method).
*/
class ParamSet : public ParamBase {
public:
    /// Constructor with ParamSet name
    ParamSet(const char* name) : ParamBase(0, name) {}
    /// Constructor with ParamSet name
    ParamSet(ParamSet* parent, const char* name) : ParamBase(parent, name) {}
    /// Set a value of a Param with the given name
    template<typename T>
    void set(const char* name, T value);
    /// Find a Param with the given name (throws if not found and abort true)
    ParamBase* find(const char* name, bool abort = true) const;
    /// Argument map for set method
    typedef std::map<const char*, const char*, StrCompare> ArgMap;
    /// Set all parameters from an argument map
    void set(const ArgMap& arg_map, bool abort = true);
    /// Set new eol and return current one
    static std::string set_eol(const std::string& eol);
    /// Stream out with prefix "name?action=set&"
    std::ostream& writeln(std::ostream& str);
    /// Stream out content with eol=newline and return resulting string
    std::string info() const;
    /// Reset current and backup to default
    virtual void reset();
    /// Revert current value from backup
    virtual void revert();
    /// Checkpoint, i.e. save current value in backup
    virtual void checkpt();
    /// true if any member changed since the last checkpoint
    virtual bool changed() const;
    /// copy all members from the other param_set
    virtual void copy_value(const ParamSet& param_set);
    /// copy all members from the other param_set that changed
    virtual void copy_changed(const ParamSet& param_set);
    /// virtual destructor for a polymorphic class
    virtual ~ParamSet() {}
    /// Friend streaming operator
    friend std::ostream& operator<<(std::ostream& str, const ParamSet& param_set);
    /// initialize from string (sets default, backup and current)
    void init(const char* line, ParamSet* parent, int index);
protected:
    /// Default constructor is protected to enable array members
    ParamSet() : ParamBase(0, "") {}
private:
    friend class ParamBase;
    /// write values to a stream
    virtual void write(std::ostream& str) const { str << *this; }

    typedef std::map<const char*, ParamBase*, StrCompare> ParamMap;
    ParamMap            _map;
    static std::string  _eol;
    static const char   _sep = '&';
    static const char   _equ = '=';

    ParamSet(const ParamSet&);  // private and unimplemented copy constructor
    ParamSet& operator=(const ParamSet&); // private and unimplemented assignment operator
    /// copy all members from the other param_set
    virtual void copy_value(const ParamBase* param_base);
    // To add a Param to this ParamSet
    void add(ParamBase* param);
    // this should never be called on ParamSet, will throw
    virtual void set_from_string(const char* line, bool abort = true);
};

/// Param with a templated value
template<typename T>
class Param : public ParamBase {
public:
    /// Verification Functor
    typedef boost::function<bool (T)> VerifyFun;
    /// Constructor with Param name and pointer to parent ParamSet
    /** @param parent   pointer to parent container
        @param name     parameter name
        @param value    initial value
        @param verify   optional verification functor, must return true if value is valid
    */
    Param(ParamSet* parent, const char* name, T value, VerifyFun verify = VerifyTrue()) : 
          ParamBase(parent, name), _value(value), _backup(value), _default(value), _verify(verify ? verify : VerifyTrue()) {
    }
    /// const accessor (so that value cannot be changed)
    const T& get() const { return _value; }
    /// accessor, so that you can get value directly
    /** This operator make it possible to write @verbatim
            Param<int> x;
            int y = x;  @endverbatim */
    operator T() const { return _value; }  
    /// assignment operator, returns const reference.
    /** Since the operator returns const T&, you can do this: a = b =c, but
        you cannot do this: (a = b) = c (why would you want to?)    */
    const T& operator=(const T& value);
    /// assignment, but with optional abort control
    void set(const T& value, bool abort = true);
    /// copy a value from another Param
    void copy_value(const ParamBase*);
    /// Verify if value held is correct
    virtual bool verify() const { return _verify(_value); }
    /// Verify if value passed as argument is correct
    virtual bool verify(T value) const { return _verify(value); }
    /// Reset current to default
    virtual void reset() { _value = _default; }
    /// Revert current value from backup
    virtual void revert() { _value = _backup; }
    /// Checkpoint, i.e. save current value in backup
    virtual void checkpt() { _backup = _value; }
    /// true if value changed compared to last checkpoint
    virtual bool changed() const { return _backup != _value; }
    /// Virtual destructor to make sure Param is polymorphic
    virtual ~Param() {}
private:
    // default verificator, always returns true
    struct VerifyTrue {
        bool operator()(T) const { return true;}
    };
    T           _value;
    T           _backup;
    T           _default;
    VerifyFun   _verify;
    /// Set value for the concrete type, reading from string
    virtual void set_from_string(const char* value, bool abort = true);
    /// write value to a stream
    virtual void write(std::ostream& str) const { str << _value; }
    /// initialize from string
    virtual void init(const char* value, ParamSet* parent, int index);
    friend bool operator==(Param<std::string>& str, const char* ptr);
};    

/// Functor to verify if the input is in a given range and, optionally, right parity
class VerifyRange {
public:
    /// Verifies if number is [min..max] (inclusive) and (optionally) if is a multiple of another
    /** If mult is > 0, checks if number is a multiple of mult.
        If mult is <=0 or absent, that checked is skipped.
    */
    VerifyRange(int min, int max, int mult = -1) : _min(min), _max(max), _mult(mult) {}
    /// return true if value is in the range and (optionally) has right parity
    bool operator()(int n) const { return n >= _min && n <= _max && (_mult <= 0 || (n % _mult == 0)); }
private:
    int _min;
    int _max;
    int _mult;
};

/// Functor to verify if input is one of possible values, works for strings and ints
class VerifyEnum {
public:
    /// Up to four strings can be provided directly in the constructor
    VerifyEnum(const char* a0, const char* a1 = 0, const char* a2 = 0, const char* a3 = 0) {
        _strings.insert(a0);
        if (a1) _strings.insert(a1);
        if (a2) _strings.insert(a2);
        if (a3) _strings.insert(a3);
    }
    /// Up to four ints can be provided directly in the constructor
    VerifyEnum(int a0, int a1, int a2, int a3) {
        _ints.insert(a0);
        _ints.insert(a1);
        _ints.insert(a2);
        _ints.insert(a3);
    }
    /// Up to four ints can be provided directly in the constructor
    VerifyEnum(int a0, int a1, int a2) {
        _ints.insert(a0);
        _ints.insert(a1);
        _ints.insert(a2);
    }
    /// Up to four ints can be provided directly in the constructor
    VerifyEnum(int a0, int a1) {
        _ints.insert(a0);
        _ints.insert(a1);
    }
    /// Up to four ints can be provided directly in the constructor
    VerifyEnum(int a0) {
        _ints.insert(a0);
    }
    /// If more then four possible values are needed, put them in a set
    VerifyEnum(const std::set<std::string>& strings) : _strings(strings) {}
    /// If more then four possible values are needed, put them in a set
    VerifyEnum(const std::set<int>& ints) : _ints(ints) {}
    /// return true if value is one of possible values
    bool operator()(std::string& value) const { 
        return _strings.find(value) != _strings.end(); 
    }
    /// return true if value is one of possible values
    bool operator()(int value) const { 
        return _ints.find(value) != _ints.end(); 
    }
private:
    std::set<std::string> _strings;
    std::set<int>         _ints;
};

/// Functor to verify IP dot notation string
class VerifyDots {
public:
    /// constructor, normal IP number requires 3 dots
    VerifyDots(int dot_count = 3, bool has_port = false) : _dot_count(dot_count),
                                                           _has_port(has_port) {}
    /// returns true if this is an IP dot address
    bool operator()(const std::string& address) const { return operator()(address.c_str()); }
    bool operator()(const char* address) const;
private:
    int     _dot_count;
    bool    _has_port;
};

/// Streams Param in the form name=value
template<typename T>
std::ostream& operator<<(std::ostream& str, const Param<T>& param) {
    str << param.name() << "=" << param.get();
    return str;
}

template<typename T>
void ParamSet::set(const char* name, T value) {
    Param<T>* param = dynamic_cast< Param<T>* >(find(name));
    if (!param) 
        throw Exception(Exception::PROGRAM_FATAL, __PRETTY_FUNCTION__, __FILE__, __LINE__, "unable to set %s to a value", name);
    *param = value;
}

template<typename T>
const T& Param<T>::operator=(const T& value) {
    set(value);
    return _value;
}

template<typename T>
void Param<T>::copy_value(const ParamBase* param_base) {
    const Param<T>* param = dynamic_cast< const Param<T>* >(param_base);
    if (!param) 
        throw Exception(Exception::PROGRAM_FATAL, __PRETTY_FUNCTION__, __FILE__, __LINE__, "unable to set %s to a value", name());
    *this = param->_value;
}

template<typename T>
void Param<T>::set(const T& value, bool abort) {
    if (verify(value)) {
        _value = value;
    } else if (abort)
        throw Exception(Exception::USER_ERROR, NULL, __FILE__, __LINE__, "invalid value for argument %s", name());
}

template<typename T>
void Param<T>::set_from_string(const char* value, bool abort) {
    T x;
    try {
        x = convert<T, const char*>(value, abort);
    } catch (Exception& ex) {
        ex.append(", for parameter %s", name());
        throw;
    }
    set(x, abort); 
}

/** string comparison operator is necessary, so that we can write
        Param<string> x;
        if (x == "something") ...
*/
inline bool operator==(Param<std::string>& str, const char* ptr) {
    return strcmp(str._value.c_str(), ptr) == 0;
}

template<typename T>
void Param<T>::init(const char* value, ParamSet* /* parent */, int /* index */) {
    set_from_string(value);
    _backup  = _value;
    _default = _value;
}
}
#endif
