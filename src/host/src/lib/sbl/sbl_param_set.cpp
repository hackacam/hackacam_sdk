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
#include <cstdio>
#include <sstream>
#include <iomanip>
#include "sbl_exception.h"
#include "sbl_logger.h"
#include "sbl_param_set.h"

#define ERROR_IF(cond, format, ...) \
    do { \
        if (cond) throw Exception(Exception::USER_ERROR, NULL, __FILE__, __LINE__, format, ##__VA_ARGS__);  \
    } while (0)

namespace SBL {

/* ----------- Specializations of the global 'convert' function -------- */
template<>
int convert<int, const char*>(const char* value, bool abort) {
    if (!abort && !value)
        return 0;
    ERROR_IF(!value, "missing value");
    char* end_ptr;
    int n = strtol(value, &end_ptr, 10);
    ERROR_IF(abort && (*end_ptr || end_ptr == value), "error converting string '%s' to integer", value);
    return n;
}


template<>
bool convert<bool, const char*>(const char* value, bool abort) {
    if (!abort && !value)
        return 0;
    ERROR_IF(!value, "missing value");
    if (strcmp(value, "true") == 0)
        return true;
    if (strcmp(value, "false") == 0)
        return false;
    char* end_ptr;
    int n = strtol(value, &end_ptr, 10);
    ERROR_IF(abort && (*end_ptr || end_ptr == value || !(n == 0 || n == 1)), "error converting string '%s' to bool", value);
    return n;
}

template<>
std::string convert<std::string, const char*>(const char* value, bool abort) {
    if (!abort && !value)
        return "";
    ERROR_IF(!value, "missing value");
    return std::string(value);
}

template<>
std::string convert<std::string, int>(int n, bool /* abort */) {
    char buffer[20];
    snprintf(buffer, sizeof buffer, "%d", n);
    return buffer;
}

template<>
double convert<double, const char*>(const char* value, bool abort) {
    if (!abort && !value)
        return 0.0;
    ERROR_IF(!value, "missing value");
    char* end_ptr;
    double d = strtod(value, &end_ptr);
    ERROR_IF(abort && (*end_ptr && end_ptr == value), "error converting string '%s' to double", value);
    return d;
}

/* ------------------------------ ParamBase methods ------------------------------ */
// stream out a parameter as name=value pair
std::ostream& operator<<(std::ostream& str, const ParamBase& param) {
    str << param.name() << '=';
    param.write(str);
    return str;
}

void ParamBase::add_index(int n) {
    _name += std::string("[") + convert<std::string, int>(n) + std::string("]");
}

ParamBase::ParamBase(ParamSet* parent, const char* name) : _name(name) {
    if (parent)
        parent->add(this);
}

/* ------------------------------ ParamSet methods ------------------------------ */
std::string ParamSet::_eol("\r\n");

// find a parameter with a given name or throw
ParamBase* ParamSet::find(const char* param_name, bool abort) const {
    ParamMap::const_iterator n = _map.find(param_name);
    if (n == _map.end()) {
        ERROR_IF(abort, "invalid argument %s for command %s", param_name, name());
        return NULL;
    }
    return n->second;
}

// add a parameter to this parameter set
void ParamSet::add(ParamBase* param) {
    _map[param->name()] = param;
}

// stream the whole parameter set
std::ostream& operator<<(std::ostream& str, const ParamSet& param_set) {
    for (ParamSet::ParamMap::const_iterator n = param_set._map.begin(); n != param_set._map.end(); ++n) {
        str << *(n->second) << ParamSet::_eol;
    }
    return str;
}

// write a single line suitable for execution by cgi server
std::ostream& ParamSet::writeln(std::ostream& str) {
    std::string nm(name());
    size_t bracket = nm.find('[');
    if (bracket != std::string::npos)
        nm = nm.substr(0, bracket);
    str << nm << "?action=set&" << *this;
    str.seekp((int) str.tellp() - _eol.length());
    str << std::endl;
    return str;
}

// get info in the form name=value,name=value,name=value...
std::string ParamSet::info() const {
    std::stringstream str;
    std::string eol(",");
    eol = set_eol(eol);
    str << *this;
    str.seekp((int) str.tellp() - _eol.length());
    set_eol(eol);
    return str.str();
}

// line is expected to be in name=value&name=value format 
void ParamSet::init(const char* line, ParamSet* parent, int index) {
    if (index >= 0)
        add_index(index);
    if (parent)
        parent->add(this);
    if (!line) 
        return;
    char buffer[strlen(line) + 1];
    char* query = strcpy(buffer, line);
    while (query && *query) {
        char* sep = strchr(query, _sep);
        if (sep)
            *sep++ = '\0';
        char* value = strchr(query, _equ);
        ERROR_IF(!value || !*(value + 1), "missing value for parameter %s", query);
        *value++ = '\0';
        find(query)->init(value, 0, -1);
        query = sep;
    }
}

void ParamSet::set_from_string(const char* /* line */, bool /* abort */ ) {
    throw Exception(Exception::PROGRAM_ERROR, __PRETTY_FUNCTION__, __FILE__, __LINE__, "illegal call to ParamSet::set_from_string()");
}

std::string ParamSet::set_eol(const std::string& eol) { 
    std::string new_eol(eol); 
    _eol.swap(new_eol); 
    return new_eol; 
}

void ParamSet::copy_changed(const ParamSet& param_set) {
    for (ParamMap::const_iterator it = param_set._map.begin(); it != param_set._map.end(); ++it) {
        if (it->second->changed()) 
            find(it->first)->copy_value(it->second);
    }
}

void ParamSet::copy_value(const ParamSet& param_set) {
    for (ParamMap::const_iterator it = param_set._map.begin(); it != param_set._map.end(); ++it) {
        find(it->first)->copy_value(it->second);
    }
}

void ParamSet::copy_value(const ParamBase* param_base) {
    const ParamSet* param_set = dynamic_cast<const ParamSet*>(param_base);
    if (!param_set) 
        throw Exception(Exception::PROGRAM_FATAL,
                        __PRETTY_FUNCTION__, __FILE__, __LINE__, "unable to set %s to a value", name());
    copy_value(*param_set);
}

/* ---------- set, reset and revert for ParamSet ------------------------ */

// set a parameter set from an argument map, ignoring unknown arguments or missing values
void ParamSet::set(const ArgMap& arg_map, bool abort) {
    for (ArgMap::const_iterator it = arg_map.begin(); it != arg_map.end(); ++it) {
        ParamBase* param = find(it->first, abort);
        if (param)
            param->set_from_string(it->second, abort); 
    }
}        


// reset all parameters in parameter set
void ParamSet::reset() {
    for (ParamMap::iterator it = _map.begin(); it != _map.end(); ++it) {
        find(it->first)->reset();
    }
}        

// revert all parameters in parameter set
void ParamSet::revert() {
    for (ParamMap::iterator it = _map.begin(); it != _map.end(); ++it) {
        find(it->first)->revert();
    }
}        

// checkpoint all parameters in parameter set
void ParamSet::checkpt() {
    for (ParamMap::iterator it = _map.begin(); it != _map.end(); ++it) {
        find(it->first)->checkpt();
    }
}        

// check if anything changed
bool ParamSet::changed() const {
    for (ParamMap::const_iterator it = _map.begin(); it != _map.end(); ++it) {
        if (find(it->first)->changed())
            return true;
    }
    return false;
}        


/* ----------- Specializations of the ParamSet::set  (otherwise dynamic_cast won't work)  -------- */
template<>
void ParamSet::set<const char*>(const char* name, const char* value) {
    set(name, std::string(value));
}

template<>
void ParamSet::set<char*>(const char* name, char* value) {
    set(name, std::string(value));
}

/* ----------- Verify IP address dot notation ---------------------------------------------------- */
bool VerifyDots::operator()(const char* s) const {
    int dots = 0;
    if (!isdigit(*s))       // must start with digit
        return false;
    while (*++s) {
        if (*s == ':' && _has_port)
            break;
        if (*s == '.') {
            dots++;
            // dot cannot follow dot and cannot be last
            if (*(s + 1) == '.' || *(s + 1) == '\0' || *(s + 1) == ':')
                return false;
        } else {
            // if it not a dot, it must be digit
            if (!isdigit(*s))
                return false;
        }
    }
    if (dots != _dot_count)
        return false;
    if (*s == '\0')
        return !_has_port;
    if (*s++ != ':')
        return false;
    do {
        if (!isdigit(*s))
            return false;
    } while (*++s);
    return true;
}

}
