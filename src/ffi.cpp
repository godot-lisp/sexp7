#include "ffi.hpp"
#include "debug_macros.hpp"
#include "ffi_macros.hpp"
#include "sx7_callable.hpp"
#include "sx7_object.hpp"

#include <godot_cpp/classes/performance.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <vector>

#define VARIANT_TYPE_TAG 0

#define EXPECT_VARIANT_ARG(n) EXPECT_ARG(n, variant_value(_arg), is_variant, "Variant")

namespace godot {
s7_pointer variant_to_string(s7_scheme *sc, s7_pointer args) {
  auto v = variant_value(s7_car(args));
  auto str = "<variant " + v->stringify() + ">";
  return godot_string_to_scheme_string(sc, str);
}

s7_pointer variant_free([[maybe_unused]] s7_scheme *sc, s7_pointer obj) {
  WATCH(obj);
  delete variant_value(obj);
  return s7_unspecified(sc);
}

bool is_variant(s7_pointer arg) {
  return s7_is_c_object(arg) && s7_c_object_type(arg) == VARIANT_TYPE_TAG;
}

s7_pointer g_is_variant(s7_scheme *sc, s7_pointer args) {
  auto arg = s7_car(args);
  if (is_variant(arg)) {
    return s7_make_integer(sc, variant_value(arg)->get_type());
  }
  return s7_make_boolean(sc, false);
}

s7_pointer make_variant_object(s7_scheme *sc, const Variant &v) {
  return s7_make_c_object(sc, VARIANT_TYPE_TAG, new Variant(v));
}

s7_pointer variant_to_scheme(s7_scheme *sc, const Variant &v) {
  WATCH_VARIANT(v);
  switch (v.get_type()) {
    case Variant::NIL:
      return s7_unspecified(sc);
    case Variant::BOOL:
      return s7_make_boolean(sc, v);
    case Variant::INT:
      return s7_make_integer(sc, v);
    case Variant::FLOAT:
      return s7_make_real(sc, v);
    case Variant::STRING_NAME:
      return s7_make_symbol(sc, v.stringify().utf8());
    case Variant::OBJECT: {
      const Object *obj = v;
      auto scheme_object = dynamic_cast<const SX7Object *>(obj);
      if (scheme_object != nullptr && scheme_object->belongs_to(sc)) {
        return scheme_object->get_scheme_ptr();
      }
      return make_variant_object(sc, v);
    }
    default:
      return make_variant_object(sc, v);
      //    case Variant::STRING:       // given the performance implications, better to
      //    convert strings explicitly
      //      break;
      //    case Variant::VECTOR2:
      //      break;
      //    case Variant::VECTOR2I:
      //      break;
      //    case Variant::RECT2:
      //      break;
      //    case Variant::RECT2I:
      //      break;
      //    case Variant::VECTOR3:
      //      break;
      //    case Variant::VECTOR3I:
      //      break;
      //    case Variant::TRANSFORM2D:
      //      break;
      //    case Variant::VECTOR4:
      //      break;
      //    case Variant::VECTOR4I:
      //      break;
      //    case Variant::PLANE:
      //      break;
      //    case Variant::QUATERNION:
      //      break;
      //    case Variant::AABB:
      //      break;
      //    case Variant::BASIS:
      //      break;
      //    case Variant::TRANSFORM3D:
      //      break;
      //    case Variant::PROJECTION:
      //      break;
      //    case Variant::COLOR:
      //      break;
      //    case Variant::NODE_PATH:
      //      break;
      //    case Variant::RID:
      //      break;
      //    case Variant::CALLABLE:
      //      break;
      //    case Variant::SIGNAL:
      //      break;
      //    case Variant::DICTIONARY:
      //      break;
      //    case Variant::ARRAY:
      //      break;
      //    case Variant::PACKED_BYTE_ARRAY:
      //      break;
      //    case Variant::PACKED_INT32_ARRAY:
      //      break;
      //    case Variant::PACKED_INT64_ARRAY:
      //      break;
      //    case Variant::PACKED_FLOAT32_ARRAY:
      //      break;
      //    case Variant::PACKED_FLOAT64_ARRAY:
      //      break;
      //    case Variant::PACKED_STRING_ARRAY:
      //      break;
      //    case Variant::PACKED_VECTOR2_ARRAY:
      //      break;
      //    case Variant::PACKED_VECTOR3_ARRAY:
      //      break;
      //    case Variant::PACKED_COLOR_ARRAY:
      //      break;
      //    case Variant::VARIANT_MAX:
      //      break;
  }
}

Variant scheme_to_variant(s7_scheme *sc, s7_pointer arg) {
  if (s7_is_integer(arg)) {
    return s7_integer(arg);
  }
  if (s7_is_real(arg)) {
    return s7_real(arg);
  }
  if (s7_is_string(arg)) {
    return scheme_string_to_godot_string(arg);
  }
  if (s7_is_boolean(arg)) {
    return s7_boolean(sc, arg);
  }
  if (is_variant(arg)) {
    return *variant_value(arg);
  }
  if (s7_is_unspecified(sc, arg)) {
    return {};
  }
  if (s7_is_procedure(arg)) {
    return Callable(memnew(SX7Callable(sc, arg, false)));
  }
  if (s7_is_symbol(arg)) {
    return StringName(s7_symbol_name(arg));
  }
  if (s7_is_character(arg)) {
    return s7_character(arg);
  }
  WATCH(arg);
  return memnew(SX7Object(sc, arg));
}

s7_pointer g_make_variant(s7_scheme *sc, s7_pointer args) {
  auto arg = s7_car(args);
  WATCH(arg);
  return make_variant_object(sc, scheme_to_variant(sc, arg));
}

s7_pointer g_variant_type_to_string(s7_scheme *sc, s7_pointer args) {
  BEGIN_ARGS();
  EXPECT_INT_ARG(variant_type);
  return godot_string_to_scheme_string(sc, UtilityFunctions::type_string(variant_type));
}

s7_pointer g_variant_string(s7_scheme *sc, s7_pointer args) {
  BEGIN_ARGS();
  EXPECT_VARIANT_ARG(variant);
  if (variant->get_type() == Variant::Type::STRING) {
    return godot_string_to_scheme_string(sc, *variant);
  }
  return godot_string_to_scheme_string(sc, variant->stringify());
}

template <typename C>
C collect_variants_into(C result, s7_scheme *sc, s7_pointer vars) {
  while (s7_is_pair(vars)) {
    result.push_back(scheme_to_variant(sc, s7_car(vars)));
    vars = s7_cdr(vars);
  }
  return result;
}

std::vector<Variant> collect_variants(s7_scheme *sc, s7_pointer args) {
  return collect_variants_into(std::vector<Variant>(), sc, args);
}

std::vector<const Variant *> collect_pointers(const std::vector<Variant> &vargs) {
  auto pointers = std::vector<const Variant *>(vargs.size());
  for (size_t i = 0; i < vargs.size(); i++) {
    pointers[i] = &vargs[i];
  }
  return pointers;
}

s7_pointer variant_call_with_receiver(s7_scheme *sc, s7_pointer receiver, s7_pointer args) {
  BEGIN_ARGS();
  EXPECT_SYMBOL_ARG(method);

  GDExtensionCallError error;
  Variant ret;

  WATCH(receiver);
  if (s7_is_pair(args)) {
    auto vargs = collect_variants(sc, args);
    auto pointers = collect_pointers(vargs);
    variant_value(receiver)->callp(s7_symbol_name(method),
        pointers.data(),
        static_cast<int>(vargs.size()),
        ret,
        error);
  } else {
    variant_value(receiver)->callp(s7_symbol_name(method), nullptr, 0, ret, error);
  }

  if (error.error != GDEXTENSION_CALL_OK) {
    return s7_error(sc,
        s7_make_symbol(sc, "godot"),
        godot_string_to_scheme_string(sc, UtilityFunctions::error_string(error.error)));
  }
  return variant_to_scheme(sc, ret);
}

s7_pointer g_variant_call(s7_scheme *sc, s7_pointer args) {
  WATCH(args);

  s7_pointer receiver = s7_car(args);
  if (!is_variant(receiver)) {
    return s7_wrong_type_arg_error(sc, __func__, 0, receiver, "Variant");
  }
  args = s7_cdr(args);
  return variant_call_with_receiver(sc, receiver, args);
}

static s7_pointer g_class_db(s7_scheme *sc, s7_pointer args) {
  return make_variant_object(sc, ClassDBSingleton::get_singleton());
}

s7_pointer variant_ref_1(s7_scheme *sc, s7_pointer receiver, s7_pointer arg) {
  if (s7_is_symbol(arg)) {
    bool valid;
    auto name = s7_symbol_name(arg);
    auto r = variant_value(receiver)->get_named(name, valid);
    if (!valid) {
      return nullptr;
    }
    return variant_to_scheme(sc, r);
  }
  if (s7_is_integer(arg)) {
    bool valid;
    bool oob;
    auto idx = s7_integer(arg);
    auto r = variant_value(receiver)->get_indexed(idx, valid, oob);
    if (!valid || oob) {
      return nullptr;
    }
    return variant_to_scheme(sc, r);
  }
  if (s7_is_list(sc, arg)) {
    return variant_call_with_receiver(sc, receiver, arg);
  }
  return nullptr;
}

s7_pointer variant_ref(s7_scheme *sc, s7_pointer args) {
  WATCH(args);

  s7_pointer res = s7_car(args);
  args = s7_cdr(args);

  if (variant_value(res)->get_type() == Variant::CALLABLE) {
    const Callable &callable = *variant_value(res);
    return variant_to_scheme(sc, callable.callv(collect_variants_into(Array(), sc, args)));
  }

  while (args != s7_nil(sc)) {
    auto arg = s7_car(args);
    res = variant_ref_1(sc, res, arg);
    if (res == nullptr) {
      return s7_unspecified(sc);
    }
    args = s7_cdr(args);
  }
  return res;
}

bool variant_set_1(s7_pointer receiver, s7_pointer arg, const Variant &value) {
  if (s7_is_symbol(arg)) {
    bool valid;
    auto name = s7_symbol_name(arg);
    variant_value(receiver)->set_named(name, value, valid);
    return valid;
  } else if (s7_is_integer(arg)) {
    bool valid;
    bool oob;
    auto idx = s7_integer(arg);
    variant_value(receiver)->set_indexed(idx, value, valid, oob);
    return valid && !oob;
  } else {
    return false;
  }
}

s7_pointer variant_set(s7_scheme *sc, s7_pointer args) {
  auto next_args = args;
  auto receiver = s7_car(next_args);
  next_args = s7_cdr(next_args);

  while (s7_cdr(next_args) != s7_nil(sc) && s7_cddr(next_args) != s7_nil(sc)) {
    auto arg = s7_car(next_args);
    receiver = variant_ref_1(sc, receiver, arg);
    if (receiver == nullptr) {
      return s7_unspecified(sc);
    }
    next_args = s7_cdr(next_args);
  }

  WATCH(receiver);
  WATCH(args);

  if (s7_cdr(next_args) == s7_nil(sc)) {
    // not enough arguments
    return s7_wrong_number_of_args_error(sc, __func__, args);
  }

  auto arg = s7_car(next_args);
  auto value = s7_cadr(next_args);
  auto variant_value = scheme_to_variant(sc, value);
  if (!variant_set_1(receiver, arg, variant_value)) {
    return s7_unspecified(sc);
  }
  return value;
}

s7_pointer variants_to_list(s7_scheme *sc, const Variant **args, int arg_count) {
  auto list = s7_nil(sc);
  while (arg_count > 0) {
    auto arg = variant_to_scheme(sc, *args[--arg_count]);
    list = s7_cons(sc, arg, list);
  }
  return list;
}

s7_pointer variant_is_equal(s7_scheme *sc, s7_pointer args) {
  auto v1 = s7_car(args); // we can assume this is a Variant
  auto v2 = s7_cadr(args);
  if (is_variant(v2)) {
    return s7_make_boolean(sc, *variant_value(v1) == *variant_value(v2));
  }
  return s7_make_boolean(sc, false);
}

s7_pointer g_make_Dictionary(s7_scheme *sc, s7_pointer args) {
  return make_variant_object(sc, Dictionary());
}

s7_pointer g_make_Color(s7_scheme *sc, s7_pointer args) {
  BEGIN_ARGS();
  EXPECT_STRING_ARG(color);
  return make_variant_object(sc, Color(s7_string(color)));
}

s7_pointer g_make_Callable(s7_scheme *sc, s7_pointer args) {
  auto f = s7_car(args);
  auto discard_return_value =
      s7_is_pair(s7_cdr(args)) ? s7_boolean(sc, s7_cadr(args)) : true;
  return make_variant_object(
      sc,
      Callable(memnew(SX7Callable(sc, f, discard_return_value))));
}

s7_pointer g_make_Vector2(s7_scheme *sc, s7_pointer args) {
  BEGIN_ARGS();
  EXPECT_REAL_ARG(x);
  EXPECT_REAL_ARG(y);
  return make_variant_object(sc, Vector2(x, y));
}

s7_pointer g_make_Vector2i(s7_scheme *sc, s7_pointer args) {
  BEGIN_ARGS();
  EXPECT_INT_ARG(x);
  EXPECT_INT_ARG(y);
  return make_variant_object(sc, Vector2i(x, y));
}

s7_pointer g_make_Rect2(s7_scheme *sc, s7_pointer args) {
  BEGIN_ARGS();
  EXPECT_REAL_ARG(x);
  EXPECT_REAL_ARG(y);
  EXPECT_REAL_ARG(w);
  EXPECT_REAL_ARG(h);
  return make_variant_object(sc, Rect2(x, y, w, h));
}

s7_pointer g_make_Rect2i(s7_scheme *sc, s7_pointer args) {
  BEGIN_ARGS();
  EXPECT_INT_ARG(x);
  EXPECT_INT_ARG(y);
  EXPECT_INT_ARG(w);
  EXPECT_INT_ARG(h);
  return make_variant_object(sc, Rect2i(x, y, w, h));
}

void print_internal(const Variant **args, GDExtensionInt arg_count) {
  static GDExtensionPtrUtilityFunction _gde_function =
      internal::gdextension_interface_variant_get_ptr_utility_function(
          StringName("print")._native_ptr(),
          2648703342);
  CHECK_METHOD_BIND(_gde_function);
  Variant ret;
  _gde_function(&ret, reinterpret_cast<GDExtensionConstVariantPtr *>(args), arg_count);
}

s7_pointer g_print(s7_scheme *sc, s7_pointer args) {
  auto vars = collect_variants(sc, args);
  auto pointers = collect_pointers(vars);
  print_internal(pointers.data(), static_cast<GDExtensionInt>(pointers.size()));
  return s7_unspecified(sc);
}

s7_pointer variant_length(s7_scheme *sc, s7_pointer args) {
  switch (const auto v = variant_value(s7_car(args)); v->get_type()) {
    case Variant::STRING:
      return s7_make_integer(sc, static_cast<const String &>(*v).length());
    case Variant::ARRAY:
      return s7_make_integer(sc, static_cast<const Array &>(*v).size());
    case Variant::PACKED_STRING_ARRAY:
      return s7_make_integer(sc, static_cast<const PackedStringArray &>(*v).size());
    case Variant::PACKED_INT32_ARRAY:
      return s7_make_integer(sc, static_cast<const PackedInt32Array &>(*v).size());
    case Variant::PACKED_INT64_ARRAY:
      return s7_make_integer(sc, static_cast<const PackedInt64Array &>(*v).size());
    case Variant::PACKED_FLOAT32_ARRAY:
      return s7_make_integer(sc, static_cast<const PackedFloat32Array &>(*v).size());
    case Variant::PACKED_FLOAT64_ARRAY:
      return s7_make_integer(sc, static_cast<const PackedFloat64Array &>(*v).size());
    case Variant::PACKED_BYTE_ARRAY:
      return s7_make_integer(sc, static_cast<const PackedByteArray &>(*v).size());
    case Variant::PACKED_VECTOR2_ARRAY:
      return s7_make_integer(sc, static_cast<const PackedVector2Array &>(*v).size());
    case Variant::PACKED_VECTOR3_ARRAY:
      return s7_make_integer(sc, static_cast<const PackedVector3Array &>(*v).size());
    case Variant::PACKED_COLOR_ARRAY:
      return s7_make_integer(sc, static_cast<const PackedColorArray &>(*v).size());
    default:
      return s7_make_boolean(sc, false);
  }
}

/*
 * TODO:
  // typed arrays
  PACKED_BYTE_ARRAY,
  PACKED_INT32_ARRAY,
  PACKED_INT64_ARRAY,
  PACKED_FLOAT32_ARRAY,
  PACKED_FLOAT64_ARRAY,
  PACKED_STRING_ARRAY,
  PACKED_VECTOR2_ARRAY,
  PACKED_VECTOR3_ARRAY,
  PACKED_COLOR_ARRAY,
*/
s7_pointer g_make_Array(s7_scheme *sc, s7_pointer args) {
  return make_variant_object(sc, collect_variants_into(Array(), sc, args));
}

void define_variant_ffi(s7 &sx7) {
  auto sc = sx7.get();
  auto variant_ty = s7_make_c_type(sc, "Variant");
  DEV_ASSERT(variant_ty == VARIANT_TYPE_TAG);

  s7_c_type_set_gc_free(sc, variant_ty, variant_free);
  s7_c_type_set_ref(sc, variant_ty, variant_ref);
  s7_c_type_set_set(sc, variant_ty, variant_set);
  s7_c_type_set_is_equal(sc, variant_ty, variant_is_equal);
  s7_c_type_set_to_string(sc, variant_ty, variant_to_string);
  s7_c_type_set_length(sc, variant_ty, variant_length);

  s7_define_function(sc,
      "Variant",
      g_make_variant,
      1,
      0,
      false,
      "(Variant obj) creates a new godot::Variant");
  s7_define_function(sc,
      "Variant?",
      g_is_variant,
      1,
      0,
      false,
      "(Variant? obj) returns the Variant type code if its argument is a godot::Variant "
      "object, #f otherwise");
  s7_define_function(sc,
      "VariantType->string",
      g_variant_type_to_string,
      1,
      0,
      false,
      "(VariantType->string variant-type-code) returns the string representation of the "
      "given Variant type code");

  s7_define_function(sc,
      "Variant->string",
      g_variant_string,
      1,
      0,
      false,
      "(Variant->string variant) returns the enclosed string if the argument is a Variant "
      "string");

  s7_define_function(sc,
      "Callable",
      g_make_Callable,
      1,
      1,
      false,
      "(Callable symbol-or-procedure (discard-return-value #t)) creates a godot::Callable "
      "which can be connected "
      "to a signal, by default, the procedure return value is discarded to avoid "
      "unnecessary processing.");
  s7_define_function(sc,
      "Color",
      g_make_Color,
      1,
      0,
      false,
      "(Color <html-color-or-color-name>) creates a godot::Color from the html or named "
      "color.");
  s7_define_function(sc,
      "Dictionary",
      g_make_Dictionary,
      0,
      0,
      false,
      "(Dictionary) creates a godot::Dictionary.");
  s7_define_function(sc,
      "Array",
      g_make_Array,
      0,
      0,
      true,
      "(Array . elements) creates a godot::Array initialized with elements.");
  s7_define_function(sc,
      "Vector2",
      g_make_Vector2,
      2,
      0,
      false,
      "(Vector2 x y) creates a godot::Vector2 with the given coordinates");
  s7_define_function(sc,
      "Vector2i",
      g_make_Vector2i,
      2,
      0,
      false,
      "(Vector2i x y) creates a godot::Vector2i with the given coordinates");
  s7_define_function(sc,
      "Rect2",
      g_make_Rect2,
      4,
      0,
      false,
      "(Rect2 real-x real-y real-width real-height) creates a godot::Rect2 with the given coordinates, width and size");
  s7_define_function(sc,
      "Rect2i",
      g_make_Rect2i,
      4,
      0,
      false,
      "(Rect2i int-x int-y int-width int-height) creates a godot::Rect2i with the given coordinates, width and size");

  s7_define_function(sc,
      "!",
      g_variant_call,
      2,
      0,
      true,
      "(! variant method-name &args) calls a method on a godot::Variant");

  s7_define_function(sc,
      "class-db",
      g_class_db,
      0,
      0,
      false,
      "(class-db) returns the Godot class database");

  s7_define_function(sc,
      "print",
      g_print,
      1,
      0,
      true,
      "(print format &args) prints to the Godot console");
}
} //namespace godot
