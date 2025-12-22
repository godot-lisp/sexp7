
#ifndef SEXP7_DEBUG_MACROS_H
#define SEXP7_DEBUG_MACROS_H

#include <s7.h>
#include <godot_cpp/variant/variant.hpp>

#define DEBUG_LOG 0

#if DEBUG_LOG

#define WATCH(e) watch_s7_value(sc, __func__, __LINE__, #e, e)
#define WATCH_VARIANT(v) watch_variant(__func__, __LINE__, #v, v)
#define LOG_CALL() (std::cout << __func__ << ":" << __LINE__ << std::endl)

s7_pointer watch_s7_value(
    s7_scheme *sc, const char *func, int line, const char *e, s7_pointer v);
const godot::Variant &watch_variant(
    const char *func, int line, const char *e, const godot::Variant &v);

#else
#define WATCH(e) 0
#define WATCH_VARIANT(v) 0
#define LOG_CALL() 0
#endif

#endif //SEXP7_DEBUG_MACROS_H
