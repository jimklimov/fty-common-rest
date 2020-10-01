/*
 *
 * Copyright (C) 2015 - 2020 Eaton
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

/*!
 * \file   utils_web.h
 * \author Alena Chernikava <AlenaChernikava@Eaton.com>
 * \author Michal Vyskocil <MichalVyskocil@Eaton.com>
 * \author Karol Hrdina <KarolHrdina@Eaton.com>
 * \brief  some helpers for web
 */
#ifndef SRC_SHARED_WEB_UTILS_H_
#define SRC_SHARED_WEB_UTILS_H_

#ifdef __cplusplus

#include <czmq.h>
#include <string>
#include <vector>
#include <mutex>
#include <list>
#include <utility>
#include <array>
#include <tuple>
#include <stdarg.h>
#include <cmath>
#include <climits>
#include <cxxtools/serializationinfo.h>
#include <fty_log.h>
//#include "utilspp.h"

#include <tnt/http.h>
#include <fty_common_utf8.h>
#include <fty_common_macros.h>

#define BIOS_SCRIPT_USER "_bios-script"

//  ###### THOSE DEFINITONS BELOW ARE PRIVATE TO http_die AND SHALL NOT BE ACCESSED DIRECTLY

/* \brief helper WebService Error struct to store all important items*/
typedef struct _wserror {
    const char* key;        ///! short key for compile time dispatch
    int http_code;          ///! http_code is HTTP reply code, use HTTP defines
    int err_code;           ///! sw internal error code
    const char* message;    ///! Message explaining the error, can contain printf like formatting chars
    int message_expansions; ///! 0 if message is a fixed string without printf variable expansions, >1 count of expansions for dynamic messages
} _WSError;

// size of _errors array, keep this up to date otherwise code won't build
static constexpr size_t _WSErrorsCOUNT = 18;

// typedef for array of errors
typedef std::array<_WSError, _WSErrorsCOUNT> _WSErrors;

// WARNING!!! - don't use anything else than %s as format parameter for .message
//
// FIXME: LEGACY NOTE: Updated _safe_die_asprintf__legacy() defined below
// supports the behavior from next paragraph, to support old code that might
// rely on it, in a way that works with modern compilers. I do not know if
// there are nowadays any use-cases that actually rely on this ambiguity.
//
// TL;DR;
// The .messages are supposed to be called with FEWER formatting arguments than defined.
// To avoid issues with going to unallocated memory, the _die_asprintf is called with
// **5** additional empty strings, which fill sefgaults for other formatting specifiers.

#define HTTP_TEAPOT 418 //see RFC2324
// Note: array index of "undefined" is 0 and will be rejected by macros below
static constexpr const _WSErrors _errors = { {
    {"undefined",                HTTP_TEAPOT,                   INT_MIN, TRANSLATE_ME_IGNORE_PARAMS ("I'm a teapot!"), 0 },
    {"internal-error",           HTTP_INTERNAL_SERVER_ERROR,    42,      TRANSLATE_ME_IGNORE_PARAMS ("Internal Server Error. %s"), 1 },
    {"not-authorized",           HTTP_UNAUTHORIZED,             43,      TRANSLATE_ME_IGNORE_PARAMS ("You are not authenticated or your rights are insufficient."), 0 },
    {"element-not-found",        HTTP_NOT_FOUND,                44,      TRANSLATE_ME_IGNORE_PARAMS ("Element '%s' not found."), 1 },
    {"method-not-allowed",       HTTP_METHOD_NOT_ALLOWED,       45,      TRANSLATE_ME_IGNORE_PARAMS ("Http method '%s' not allowed."), 1 },
    {"request-param-required",   HTTP_BAD_REQUEST,              46,      TRANSLATE_ME_IGNORE_PARAMS ("Parameter '%s' is required."), 1 },
    {"request-param-bad",        HTTP_BAD_REQUEST,              47,      TRANSLATE_ME_IGNORE_PARAMS ("Parameter '%s' has bad value. Received %s. Expected %s."), 3 },
    {"bad-request-document",     HTTP_BAD_REQUEST,              48,      TRANSLATE_ME_IGNORE_PARAMS ("Request document has invalid syntax. %s"), 1 },
    {"data-conflict",            HTTP_CONFLICT,                 50,      TRANSLATE_ME_IGNORE_PARAMS ("Element '%s' cannot be processed because of conflict. %s"), 2 },
    {"action-forbidden",         HTTP_FORBIDDEN,                51,      TRANSLATE_ME_IGNORE_PARAMS ("%s is forbidden. %s"), 2 },
    {"parameter-conflict",       HTTP_BAD_REQUEST,              52,      TRANSLATE_ME_IGNORE_PARAMS ("Request cannot be processed because of conflict in parameters. %s"), 1 },
    {"content-too-big",          HTTP_REQUEST_ENTITY_TOO_LARGE, 53,      TRANSLATE_ME_IGNORE_PARAMS ("Content size is too big, maximum size is %s."), 1 },
    {"not-found",                HTTP_NOT_FOUND,                54,      TRANSLATE_ME_IGNORE_PARAMS ("%s does not exist."), 1 },
    {"precondition-failed",      HTTP_PRECONDITION_FAILED,      55,      TRANSLATE_ME_IGNORE_PARAMS ("Precondition failed - %s"), 1 },
    {"db-err",                   HTTP_INTERNAL_SERVER_ERROR,    56,      TRANSLATE_ME_IGNORE_PARAMS ("General DB error. %s"), 1 },
    {"bad-input",                HTTP_BAD_REQUEST,              57,      TRANSLATE_ME_IGNORE_PARAMS ("Incorrect input. %s"), 1 },
    {"licensing-err",            HTTP_FORBIDDEN,                58,      TRANSLATE_ME_IGNORE_PARAMS ("Action forbidden in current licensing state. %s"), 1 },
    {"upstream-err",             HTTP_BAD_GATEWAY,              59,      TRANSLATE_ME_IGNORE_PARAMS ("Server which was contacted to fulfill the request has returned an error. %s"), 1 }
//    {.key = "undefined",                .http_code = HTTP_TEAPOT,                   .err_code = INT_MIN, .message = "I'm a teapot!", .message_expansions = 0 },
//    {.key = "internal-error",           .http_code = HTTP_INTERNAL_SERVER_ERROR,    .err_code = 42,      .message = "Internal Server Error. %s", .message_expansions = 1 },
//    {.key = "not-authorized",           .http_code = HTTP_UNAUTHORIZED,             .err_code = 43,      .message = "You are not authenticated or your rights are insufficient.", .message_expansions = 0 },
//    {.key = "element-not-found",        .http_code = HTTP_NOT_FOUND,                .err_code = 44,      .message = "Element '%s' not found.", .message_expansions = 1 },
//    {.key = "method-not-allowed",       .http_code = HTTP_METHOD_NOT_ALLOWED,       .err_code = 45,      .message = "Http method '%s' not allowed.", .message_expansions = 1 },
//    {.key = "request-param-required",   .http_code = HTTP_BAD_REQUEST,              .err_code = 46,      .message = "Parameter '%s' is required.", .message_expansions = 1 },
//    {.key = "request-param-bad",        .http_code = HTTP_BAD_REQUEST,              .err_code = 47,      .message = "Parameter '%s' has bad value. Received %s. Expected %s", .message_expansions = 3 },
//    {.key = "bad-request-document",     .http_code = HTTP_BAD_REQUEST,              .err_code = 48,      .message = "Request document has invalid syntax. %s", .message_expansions = 1 },
//    {.key = "data-conflict",            .http_code = HTTP_CONFLICT,                 .err_code = 50,      .message = "Element '%s' cannot be processed because of conflict. %s", .message_expansions = 2 },
//    {.key = "action-forbidden",         .http_code = HTTP_FORBIDDEN,                .err_code = 51,      .message = "%s is forbidden. %s", .message_expansions = 2 },
//    {.key = "parameter-conflict",       .http_code = HTTP_BAD_REQUEST,              .err_code = 52,      .message = "Request cannot be processed because of conflict in parameters. %s", .message_expansions = 1 },
//    {.key = "content-too-big",          .http_code = HTTP_REQUEST_ENTITY_TOO_LARGE, .err_code = 53,      .message = "Content size is too big, maximum size is %s", .message_expansions = 1 },
//    {.key = "not-found",                .http_code = HTTP_NOT_FOUND,                .err_code = 54,      .message = "%s does not exist.", .message_expansions = 1 },
//    {.key = "precondition-failed",      .http_code = HTTP_PRECONDITION_FAILED,      .err_code = 55,      .message = "Precondition failed - %s", .message_expansions = 1 }
    } };
#undef HTTP_TEAPOT

template <size_t N>
constexpr ssize_t
_die_idx(const char* key)
{
    static_assert(std::tuple_size<_WSErrors>::value > N, "_die_idx asked for too big N");
    return (strcmp(_errors.at(N).key, key) == 0 || strcmp(_errors.at(N).message, key) == 0) ? N: _die_idx<N-1>(key);
}

template <>
constexpr ssize_t
_die_idx<1>(const char* key)
{
    static_assert(std::tuple_size<_WSErrors>::value > 1 , "_die_idx asked for too big N");
    return (strcmp(_errors.at(1).key, key) == 0 || strcmp(_errors.at(1).message, key) == 0) ? 1: 0;
}

static int
_die_asprintf(
        char **buf,
        const char* format,
        ...) __attribute__ ((format (printf, 2, 3))) __attribute__ ((__unused__));

static int
_die_asprintf(
        char **buf,
        const char* format,
        ...)
{
    va_list args;

    va_start(args, format);
    std::string buf_str = UTF8::vajsonify_translation_string (format, args);
    va_end(args);
    size_t length = buf_str.length ();
    *buf = (char *) zmalloc (length + 1);
    strcpy (*buf, buf_str.c_str ());

    return length;
}

//  ###### THOSE DEFINITONS ABOVE ARE PRIVATE TO http_die AND SHALL NOT BE ACCESSED DIRECTLY

/*
 * \brief print valid json error and die with proper http code
 *
 * \param[in] key - the .key or .message from static list of errors
 * \param[in] ... - format arguments for .message template
 *
 * can be used as ``http_die("internal-error", e.what())`` or
 * ``http_die("Internal Server Error: %s", e.what())``
 *
 * Mistakes in key strings are compile time errors, so one can't
 * make the mistake even if he want to.
 *
 */

/* If there is a codepath that did not choose a particular Content-Type yet,
 * make sure it is JSON (e.g. die on bad permissions for getlog action) */
#define http_die_contenttype_graceful(_replyobj) \
    do { \
        if (_replyobj.getContentType() == std::string("")) { \
            _replyobj.setContentType("application/json;charset=UTF-8"); \
        } \
    } \
    while(0)

#define http_die_contenttype_brutal(_replyobj) \
    do { \
        _replyobj.setContentType("application/json;charset=UTF-8"); \
    } \
    while(0)

/* By default use this variant - our version of tntnet seems VERY BAD at
 * replacing headers in practice, so we should only add one if not present */
#define http_die_contenttype    http_die_contenttype_graceful

/* Note: Code below relies on GCC extension that a ", ##__VA_ARGS" with double
 * hash chars, expands to the args if present, or removes the comma if not.
 */

// This wrapper allows to code `http_add_error (debug, errors, "not-authorized", "");`
// which pads an empty variadic argument because C++11 requires to have one in macros
// and yet avoid formatting errors because the format string does not refer to it.
// Note that in practice it must have 3+ arguments (char** buf, int argc, msgfmt, ...)
// but to work around gcc warnings it hides msgfmt as the first variadic argument.
// Note that expandable arguments here must be "char*" strings; hopefully args[]
// definition below will complain at compile-time about incompatible arguments.
// LEGACY: Uses non-zero values of argc to pad to needed amount with empty strings
// if argn<argc because older code could rely on this:
//    _safe_die_asprintf(buf, 0, "not-authorized") => argc==0, argn==1
//    _safe_die_asprintf(buf, 0, "not-authorized", "") => argc==0, argn==2
//    _safe_die_asprintf(buf, 3, "request-param-bad", "parname", "gotval", "expval") => argc==3, argn==4
//    _safe_die_asprintf(buf, 3, "request-param-bad", "parname", "gotval") => argc==3, argn==3 (+1 generated: 4th "")
//    _safe_die_asprintf(buf, 3, "request-param-bad", "parname") => argc==3, argn==2 (+2 generated: 3rd "" + 4th "")
// If we decide to disable this failsafe, throw-switch here or re-define for a
// tested component build the alias of _safe_die_asprintf__nolegacy() below -
// that would be a bit faster too.

#define _safe_die_asprintf__legacy(buf, argc, ...) \
    do { \
        const char* args[] = { __VA_ARGS__ }; \
        const size_t argn = (sizeof(args)/sizeof(const char*)); \
        static_assert( (argn > 0), "No format and further strings passed into _safe_die_asprintf()"); \
        static_assert( (argc >= 0), "Negative count of expected vararg strings for the error message passed into _safe_die_asprintf()"); \
        switch (argc) { \
            case 0: _die_asprintf(buf, "%s", args[0]); break; \
            case 1: _die_asprintf(buf, args[0], (argn>argc)?args[1]:""); break; \
            case 2: _die_asprintf(buf, args[0], (argn>argc)?args[1]:"", (argn>argc)?args[2]:""); break; \
            case 3: _die_asprintf(buf, args[0], (argn>argc)?args[1]:"", (argn>argc)?args[2]:"", (argn>argc)?args[3]:""); break; \
            case 4: _die_asprintf(buf, args[0], (argn>argc)?args[1]:"", (argn>argc)?args[2]:"", (argn>argc)?args[3]:"", (argn>argc)?args[4]:""); break; \
            case 5: _die_asprintf(buf, args[0], (argn>argc)?args[1]:"", (argn>argc)?args[2]:"", (argn>argc)?args[3]:"", (argn>argc)?args[4]:"", (argn>argc)?args[5]:""); break; \
            default: static_assert(argc<6, "Unexpected amount of strings passed into _safe_die_asprintf()"); \
        } \
    } \
    while(0)

#define _safe_die_asprintf__nolegacy(buf, argc, ...) \
    do { \
        const char* args[] = { __VA_ARGS__ }; \
        const size_t argn = (sizeof(args)/sizeof(const char*)); \
        static_assert( (argn > 0), "No format and further strings passed into _safe_die_asprintf()"); \
        static_assert( (argc >= 0), "Negative count of expected vararg strings for the error message passed into _safe_die_asprintf()"); \
        static_assert( (argn > argc), "Too few vararg strings for the error message passed into _safe_die_asprintf()"); \
        switch (argc) { \
            case 0: _die_asprintf(buf, "%s", args[0]); break; \
            case 1: _die_asprintf(buf, args[0], args[1]); break; \
            case 2: _die_asprintf(buf, args[0], args[1], args[2]); break; \
            case 3: _die_asprintf(buf, args[0], args[1], args[2], args[3]); break; \
            case 4: _die_asprintf(buf, args[0], args[1], args[2], args[3], args[4]); break; \
            case 5: _die_asprintf(buf, args[0], args[1], args[2], args[3], args[4], args[5]); break; \
            default: static_assert(argc<6, "Unexpected amount of strings passed into _safe_die_asprintf()"); \
        } \
    } \
    while(0)

#ifndef _safe_die_asprintf
#define _safe_die_asprintf _safe_die_asprintf__legacy
#endif

#define http_die(key, ...) \
    do { \
        constexpr size_t __http_die__key_idx__ = _die_idx<_WSErrorsCOUNT-1>((const char*)key); \
        static_assert(__http_die__key_idx__ != 0, "Can't find '" key "' in list of error messages. Either add new one either fix the typo in key"); \
        char *__http_die__error_message__ = NULL; \
        _safe_die_asprintf(&__http_die__error_message__, _errors.at(__http_die__key_idx__).message_expansions, _errors.at(__http_die__key_idx__).message, ##__VA_ARGS__); \
        if (::getenv ("BIOS_LOG_LEVEL") && !strcmp (::getenv ("BIOS_LOG_LEVEL"), "LOG_DEBUG")) { \
            std::string __http_die__debug__ = {__FILE__}; \
            __http_die__debug__ += ": " + std::to_string (__LINE__); \
            reply.out() << utils::json::create_error_json(__http_die__error_message__, _errors.at(__http_die__key_idx__).err_code, __http_die__debug__); \
        } \
        else \
            reply.out() << utils::json::create_error_json(__http_die__error_message__, _errors.at(__http_die__key_idx__).err_code); \
        free(__http_die__error_message__); \
        http_die_contenttype(reply); \
        return _errors.at(__http_die__key_idx__).http_code;\
    } \
    while(0)


/**
 *  \brief http die based on _error index number
 *
 *  \param[in] idx - index to _error array
 *  \param[msg] msg - message to be printed
 *
 * idx is normalized before is used - absolute value is used for indexing, so 1
 * equals to -1.
 * If it's bigger than size of array, it's changed to 0.
 * 0 means, that there is an error hidden somewhere.
 *
 * XXX: should not we change it to http_die_be to use instance of BiosError directly?
 */
#define http_die_idx(idx, msg) \
do { \
    int64_t _idx = idx; \
    if (_idx < 0) _idx = _idx * -1; \
    if (_idx >= (int64_t)_WSErrorsCOUNT) _idx = 0; \
    if (_idx == 0) log_error("TEAPOT");\
    if (::getenv ("BIOS_LOG_LEVEL") && !strcmp (::getenv ("BIOS_LOG_LEVEL"), "LOG_DEBUG")) { \
        std::string __http_die__debug__ = {__FILE__}; \
        __http_die__debug__ += ": " + std::to_string (__LINE__); \
        reply.out() << utils::json::create_error_json(msg, _errors.at(_idx).err_code, __http_die__debug__);\
    } \
    else \
        reply.out() << utils::json::create_error_json(msg, _errors.at(_idx).err_code);\
    http_die_contenttype(reply); \
    return _errors.at(_idx).http_code;\
} \
while (0)

typedef struct _http_errors_t {
    uint32_t http_code;
    std::vector <std::tuple <uint32_t, std::string, std::string>> errors;
} http_errors_t;

#define http_add_error(debug, errors, key, ...) \
do { \
    static_assert (std::is_same <decltype (errors), http_errors_t&>::value, "'errors' argument in macro http_add_error must be a http_errors_t."); \
    constexpr size_t __http_die__key_idx__ = _die_idx<_WSErrorsCOUNT-1>((const char*)key); \
    static_assert(__http_die__key_idx__ != 0, "Can't find '" key "' in list of error messages. Either add new one either fix the typo in key"); \
    (errors).http_code = _errors.at (__http_die__key_idx__).http_code; \
    char *__http_die__error_message__ = NULL; \
    _safe_die_asprintf(&__http_die__error_message__, _errors.at(__http_die__key_idx__).message_expansions, _errors.at(__http_die__key_idx__).message, ##__VA_ARGS__); \
    (errors).errors.push_back (std::make_tuple (_errors.at (__http_die__key_idx__).err_code, __http_die__error_message__, (debug))); \
    free (__http_die__error_message__); \
} \
while (0)

#define http_die_error(errors) \
do { \
    static_assert (std::is_same <decltype (errors), http_errors_t>::value, "'errors' argument in macro http_add_error must be a http_errors_t."); \
    reply.out() << utils::json::create_error_json ((errors).errors); \
    http_die_contenttype(reply); \
    return (errors).http_code; \
} \
while (0)

/**
 * \brief general bios error
 *
 * \param[in] idx - index to _errors array
 * \param[in] message - error message to be printed
 *
 * This exception is not supposed to be created manually as it's easy to
 * misstype it. The bios_throw macro below is supposed to be used.
 *
 * Only one good case is to throw an error from low level function, euther via
 * return + message string or via db_reply_t or reply_t types.
 */
struct BiosError : std::invalid_argument {
    explicit BiosError (
            size_t idx,
            const std::string& message):
        std::invalid_argument(message),
        idx(idx)
    { }

    size_t idx;
};

/*
 * \brief get the index to error message and formatted error message
 *
 * \param[out] idx - variable name to store index
 * \param[out] str - variable name to store formatted error message
 * \param[in]  key - the .key or .message from static list of errors
 * \param[in]  ... - format arguments for .message template
 *
 * It is supposed for DB functions to return errors easily expressed in REST API
 *
 * // new low level DB API:
 * int db_foo_bar(conn, id, std::string &err) {
 *   ...
 *   if (!failed)
 *     return 0;
 *   ...
 *   int idx;
 *   bios_error_idx(idx, err, "internal-error");
 *   return -idx;
 * }
 *
 * // old low level DB API:
 * db_reply_t db_ham_spam(conn, id) {
 *   ...
 *   if (!failed)
 *     return ret;
 *   ...
 *   bios_error_idx(ret.rowid, ret.msg, "internal-error");
 *   return ret;
 * }
 *
 * // in REST API
 * int r = db_foo_bar(...);
 * if (r == 0)
 *   return HTTP_OK;
 *
 * http_die_idx(idx, message);
 *
 */

#define bios_error_idx(idx, str, key, ...) \
do { \
    static_assert (std::is_same <decltype (str), std::string>::value || std::is_same <decltype (str), std::string&>::value, "'str' argument in macro bios_error_idx must be a std::string."); \
    constexpr size_t __http_die__key_idx__ = _die_idx<_WSErrorsCOUNT-1>((const char*)key); \
    static_assert(__http_die__key_idx__ != 0, "Can't find '" key "' in list of error messages. Either add new one either fix the typo in key"); \
    char *__http_die__error_message__ = NULL; \
    _safe_die_asprintf(&__http_die__error_message__, _errors.at(__http_die__key_idx__).message_expansions, _errors.at(__http_die__key_idx__).message, ##__VA_ARGS__); \
    str = __http_die__error_message__; \
    idx = __http_die__key_idx__; \
    free (__http_die__error_message__); \
} \
while (0)

/**
 * \brief throw specified bios error
 *
 * \param[in] key - the .key or .message from static list of errors
 * \param[in] ... - format arguments for .message template
 *
 * Similar to http_die, just throws BiosError, where it can be easily 'consumed'
 * by http_die_idx macro.
 *
 */

#define bios_throw(key, ...) \
    do { \
        constexpr size_t __http_die__key_idx__ = _die_idx<_WSErrorsCOUNT-1>((const char*)key); \
        static_assert(__http_die__key_idx__ != 0, "Can't find '" key "' in list of error messages. Either add new one either fix the typo in key"); \
        char *__http_die__error_message__ = NULL; \
        _safe_die_asprintf(&__http_die__error_message__, _errors.at(__http_die__key_idx__).message_expansions, _errors.at(__http_die__key_idx__).message, ##__VA_ARGS__); \
        std::string str{__http_die__error_message__}; \
        free(__http_die__error_message__); \
        log_warning("throw BiosError{%zu, \"%s\"}", __http_die__key_idx__, str.c_str());\
        throw BiosError{__http_die__key_idx__, str}; \
    } while (0);


// General template for whether type T (a standard container) is iterable
// We deliberatelly don't want to solve this for general case (we don't need it)
// If we ever need a general is_container - http://stackoverflow.com/a/9407521
template <typename T>
struct is_iterable {
      static const bool value = false;
};

// Partial specialization for std::vector
template <typename T,typename Alloc>
struct is_iterable<std::vector <T,Alloc> > {
      static const bool value = true;
};

// Partial specialization for std::list
template <typename T,typename Alloc>
struct is_iterable<std::list <T,Alloc> > {
      static const bool value = true;
};


namespace utils {

/*!
 \brief Convert string to element identifier

 \throws std::out_of_range      When number represented by 'string' is out of <1, UINT_MAX> range
 \throws std::invalid_argument  When 'string' does not represent a number
 \return element identifier or throws
*/
uint32_t string_to_element_id (const std::string& string);

/*!
 \brief  Return an identifier for the MLM client, based on PID/pThreadID/LWP-id
 \return Provided prefix plus ID from definition above, or plus a random number.
*/
std::string generate_mlm_client_id(std::string client_name);


namespace json {

std::string jsonify (double t);

template <typename T
        , typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
std::string jsonify (T t) {
    try {
        return UTF8::escape (std::to_string (t));
    } catch (...) {
        return "";
    }
}

// TODO: doxy
// basically, these are property "jsonifyrs"; you supply any json-ifiable type pair and it creates a valid, properly escaped property (key:value) pair out of it.
// single arg version escapes and quotes were necessary (i.e. except int types...)

template <typename T
        , typename std::enable_if<std::is_convertible<T, std::string>::value>::type* = nullptr>
std::string jsonify (const T& t) {
    try {
        // check if the arg is already JSON
        std::string t_str (t);
        size_t length = t_str.length ();
        if (length >= 2 && t[0] == '{' && t[1] != '{' && t[length-2] != '}' && t[length-1] == '}') {
            log_trace (t_str.c_str ());
            return t;
        }
        return std::string ("\"").append (UTF8::escape (t)).append ("\"");
    } catch (...) {
        return "";
    }
}

template <typename T
        , typename std::enable_if<is_iterable<T>::value>::type* = nullptr>
std::string jsonify (const T& t) {
    try {
        std::string result = "[ ";
        bool first = true;
        for (const auto& item : t) {
            if (first) {
                result += jsonify (item);
                first = false;
            }
            else {
                result += ", " + jsonify (item);
            }
        }
        result += " ]";
        return result;
    } catch (...) {
        return "[]";
    }
}

template <typename S
        , typename std::enable_if<std::is_convertible<S, std::string>::value>::type* = nullptr
        , typename T
        , typename std::enable_if<std::is_convertible<T, std::string>::value>::type* = nullptr>
std::string jsonify (const S& key, const T& value) {
    return std::string (jsonify (key)).append (" : ").append (jsonify (value));
}

template <typename S
        , typename = typename std::enable_if<std::is_convertible<S, std::string>::value>::type
        , typename T
        , typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
std::string jsonify (const S& key, T value) {
    return std::string (jsonify (key)).append (" : ").append (jsonify (value));
}

template <typename S
        , typename std::enable_if<std::is_convertible<S, std::string>::value>::type* = nullptr
        , typename T
        , typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
std::string jsonify (T key, const S& value) {
    return std::string ("\"").append (jsonify (key)).append ("\" : ").append (jsonify (value));
}

template <typename T
        , typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
std::string jsonify (T key, T value) {
    return std::string ("\"").append (jsonify (key)).append ("\" : ").append (jsonify (value));
}

template <typename S
        , typename std::enable_if<std::is_convertible<S, std::string>::value>::type* = nullptr
        , typename T
        , typename std::enable_if<is_iterable<T>::value>::type* = nullptr>
std::string jsonify (const S& key, const T& value) {
    return std::string (jsonify (key)).append (" : ").append (jsonify (value));
}

template <typename S
        , typename std::enable_if<is_iterable<S>::value>::type* = nullptr
        , typename T
        , typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
std::string jsonify (T key, const S& value) {
    return std::string ("\"").append (jsonify (key)).append ("\" : ").append (jsonify (value));
}

std::string
create_error_json (const std::string& message, uint32_t code);

std::string
create_error_json (const std::string& message, uint32_t code, const std::string& debug);

std::string
create_error_json (std::vector <std::tuple<uint32_t, std::string, std::string>> messages);

} // namespace utils::json

namespace config {

const char *
get_mapping (const std::string& key);

const char *
get_path (const std::string& key);

/*!
 \brief Convert json for config call to respective zpl structure

 \param [out] roots - map of file_path to zconfig with updated values
 \param [in]  si - parser JSON document
 \param [in]  lock - the guard to ensure this function runs only once
 \throws BiosError if input parameters are wrong
*/
void
json2zpl (
        std::map <std::string, zconfig_t*> &roots,
        const cxxtools::SerializationInfo &si,
        std::lock_guard <std::mutex> &lock);

/*!
 \brief Free zpl structures allocated by json2zpl

 \param [in] roots - map of file_path to zconfig with updated values
*/
void
roots_destroy (const std::map <std::string, zconfig_t*>& roots);

} // namespace utils::config

namespace email {
    /*!
     *  \brief Add various X-Eaton-IPC headers ho zhash_t
     *   \param [in]  headers - hashmap with headers
     *   */
    void
        x_headers (zhash_t *headers);
} // namespace utils::email

} // namespace utils


#ifdef __cplusplus
extern "C" {
#endif

//  Self test of this class
void
    fty_common_rest_utils_web_test (bool verbose);

#ifdef __cplusplus
}
#endif

#endif // __cplus_plus

#endif // SRC_SHARED_WEB_UTILS_H_
