#include "sx7_callable.hpp"
#include "ffi.hpp"
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/variant.hpp>

using namespace godot;

SX7Callable::SX7Callable(s7_scheme *sc, s7_pointer f, bool discard_return_value) :
    sc(sc),
    f(s7_gc_protected(sc, f)),
    discard_return_value(discard_return_value) {
}

uint32_t SX7Callable::hash() const {
  return s7_hash_code(sc, f.get(), s7_unspecified(sc));
}

String SX7Callable::get_as_text() const {
  return scheme_object_to_godot_string(sc, f.get());
}

bool SX7Callable::compare_equal_func(const CallableCustom *a, const CallableCustom *b) {
  if (a == b) {
    return true;
  }
  auto sc1 = dynamic_cast<const SX7Callable *>(a);
  auto sc2 = dynamic_cast<const SX7Callable *>(b);
  if (sc1 == sc2) {
    return true;
  }
  if (sc1 == nullptr || sc2 == nullptr) {
    return false;
  }
  auto sc = sc1->sc;
  if (sc2->sc != sc) {
    return false;
  }
  return s7_is_equal(sc, sc1->f.get(), sc2->f.get());
}

CallableCustom::CompareEqualFunc SX7Callable::get_compare_equal_func() const {
  return &SX7Callable::compare_equal_func;
}

bool SX7Callable::compare_less_func(const CallableCustom *a, const CallableCustom *b) {
  return (void *)a < (void *)b;
}

CallableCustom::CompareLessFunc SX7Callable::get_compare_less_func() const {
  return &SX7Callable::compare_less_func;
}

bool SX7Callable::is_valid() const { return true; }

ObjectID SX7Callable::get_object() const {
  auto node = s7_name_to_value(sc, "*node*");
  if (is_variant(node)) {
    return ObjectID(variant_value(node)->operator Object *()->get_instance_id());
  }
  return {};
}

void SX7Callable::call(const Variant **args,
    int arg_count,
    Variant &return_value,
    GDExtensionCallError &return_call_error) const {
  auto fp = f.get();
  auto proc = fp;
  if (s7_is_symbol(fp)) {
    proc = s7_symbol_value(sc, fp);
  }

  if (!s7_is_procedure(proc)) {
    return_call_error.error = GDEXTENSION_CALL_ERROR_INVALID_METHOD;
    return;
  }

  auto res = s7_call_with_location(
      sc,
      proc,
      variants_to_list(sc, args, arg_count),
      __func__,
      __FILE__,
      __LINE__);
  if (!discard_return_value) {
    return_value = scheme_to_variant(sc, res);
  }
  return_call_error.error = GDEXTENSION_CALL_OK;
}
