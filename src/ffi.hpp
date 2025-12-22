#ifndef SEXP7_FFI_H
#define SEXP7_FFI_H

#include "s7.hpp"

namespace godot {
void define_variant_ffi(s7 &s7);

s7_pointer make_variant_object(s7_scheme *sc, const Variant &v);

bool is_variant(s7_pointer arg);

inline Variant *variant_value(s7_pointer variant) {
  return (Variant *)s7_c_object_value(variant);
}

s7_pointer variants_to_list(s7_scheme *sc, const Variant **args, int arg_count);

s7_pointer variant_to_scheme(s7_scheme *sc, const Variant &v);

Variant scheme_to_variant(s7_scheme *sc, s7_pointer arg);

inline s7_pointer godot_string_to_scheme_string(s7_scheme *sc, const String &s) {
  auto utf8 = s.utf8();
  return s7_make_string_with_length(sc, utf8, utf8.length());
}

inline auto scheme_string_to_godot_string(s7_pointer s) {
  return String::utf8(s7_string(s), static_cast<int>(s7_string_length(s)));
}

inline auto scheme_object_to_godot_string(s7_scheme *sc, s7_pointer o) {
  auto str = s7_object_to_string(sc, o, false);
  return scheme_string_to_godot_string(str);
}
} //namespace godot

#endif //SEXP7_FFI_H
