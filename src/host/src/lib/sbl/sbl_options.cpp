#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <iomanip>
#include "sbl_options.h"

namespace SBL {

OptionBase::OptionBase(OptionSet* parent, const char* name, const char* help) : 
    _parent(parent), _name(name), _help(help), _used(false)
{
    parent->_options.push_back(this);
}

OptionSet::OptionSet(const char* header, const char* footer) :
    _prog_name(0), _header(header), _footer(footer), _help_req(false)
{
    _error[0] = '\0';
}

void OptionSet::parse(int argc, char* argv[]) {
    _prog_name = strrchr(argv[0], '/');
    _prog_name = _prog_name ? _prog_name + 1 : argv[0];
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            _pos_args.push_back(argv[i]);
            continue;
        }
        OptionBase* option = find_non_ambig(argv[i]);
        if (_error[0] || _help_req)
            return;
        if (option->used()) {
            write_error("duplicate option %s", option->name());
            return;
        }
        if (i >= argc - 1) {
            write_error("missing value for argument '%s'", argv[i]);
            return;
        }
        option->set_from_string(argv[++i]);
        option->_used = true;
    }
}

OptionBase* OptionSet::find(const char* name) {
    for (Options::const_iterator it = _options.begin(); it != _options.end(); ++it)
        if (strcmp((*it)->name(), name) == 0)
            return *it;
    return 0;
}

std::ostream& operator<<(std::ostream& str, const OptionSet& options) {
    for (OptionSet::Options::const_iterator it = options._options.begin(); it != options._options.end(); ++it)
        (*it)->write(str);
    return str;
}


/* Return option which name best matches 'name', which must start with one or two '-' (which are removed).
 * Each option name is compared to 'name' and the length of initial segment that matches
 * is computed. The option with the longest match is returned. If two options have the
 * same match length, solution is ambigous and error is logged. If no match is found,
 * unknown option error is logged */
OptionBase* OptionSet::find_non_ambig(const char* name) {
    if (*++name == '-')
        name++;
    OptionBase* option = NULL;
    int best_match = match_len(name, "help");
    _help_req = best_match > 0;
    for (Options::const_iterator it = _options.begin(); it != _options.end(); ++it) { 
        int len = match_len(name, (*it)->name());
        if (len > best_match) {
            option = *it;
            best_match = len;
            _help_req = false;
        } else if (len == best_match) {
            option = NULL;
            _help_req = false;
        }
    }
    if (!option && !_help_req)
        write_error("%s option %s", best_match ? "ambigous" : "unrecognized", name);
    return option;
}

/* return length of identical inital portions of s1 and s2 */
int OptionSet::match_len(const char* s1, const char* s2) {
    int n = 0;
    while (*s1 && *s2 && *s1++ == *s2++)
        n++;
    return n;
}

const char* OptionSet::type_name(OptionBase* option) {
    if (dynamic_cast< Option<int>*    >(option))   return "int";
    if (dynamic_cast< Option<double>* >(option))   return "double";
    if (dynamic_cast< Option<bool>*   >(option))   return "bool";
    if (dynamic_cast< Option<String>* >(option))   return "string";
    write_error("unknown option type for option '%s'", option->name());
    return 0;
}

bool OptionSet::help(std::ostream& str) {
    if (!_help_req && !error())
        return false;
    if (error())
        str << "Error: " << error() << std::endl << std::endl;
    if (_header)
        str << _header << std::endl;
    str << "Usage: " << _prog_name << " [-option(s)]" << std::endl;
    for (Options::iterator it = _options.begin(); it != _options.end(); ++it) {
        OptionBase* option = (*it);
        str << "    -" << option->name();
        const char* type = type_name(option);
        int width = strlen(option->name()) + 4;
        width += strlen(type) + 3;
        str << " <" << type << ">";
        width = width >= HELP_SPACE ? 1 : HELP_SPACE - width;
        str << std::setw(width) << ' ';
        str << option->help() << " (";
        option->deflt(str);
        str << ")" << std::endl;
    }
    str << "    -help" << std::setw(HELP_SPACE - 8) << ' ' << "print this message" << std::endl;
    if (_footer)
        str << _footer << std::endl;
    return true;
}

void OptionSet::write_error(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(_error, sizeof _error, format, args);
    va_end(args);
}

void OptionBase::write_error(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(_parent->_error, sizeof _parent->_error, format, args);
    va_end(args);
}


template<>
void Option<bool>::set_from_string(const char* value) {
    if (strcmp(value, "true") == 0 || strcmp(value, "1") == 0)
        _value = true;
    else if (strcmp(value, "false") == 0 || strcmp(value, "0") == 0)
        _value = false;
    else
        write_error("error converting string '%s' to bool", value);
}

template<>
void Option<OptionSet::String>::set_from_string(const char* value) {
    _value = value;
}

template<>
void Option<int>::set_from_string(const char* value) {
    char* end_ptr;
    _value = strtol(value, &end_ptr, 10);
    if (*end_ptr || end_ptr == value)
        write_error("error converting string '%s' to integer", value);
}

template<>
void Option<double>::set_from_string(const char* value) {
    char* end_ptr;
    _value = strtod(value, &end_ptr);
    if (*end_ptr || end_ptr == value)
        write_error("error converting string '%s' to double", value);
}



}    


