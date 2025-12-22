#ifndef SX7_CALLABLE_H
#define SX7_CALLABLE_H

#include "s7.hpp"
#include <godot_cpp/variant/callable_custom.hpp>

namespace godot {
class SX7Callable : public CallableCustom {
public:
  SX7Callable(s7_scheme *sc, s7_pointer f, bool discard_return_value);

  [[nodiscard]] uint32_t hash() const override;

  [[nodiscard]] String get_as_text() const override;

  static bool compare_equal_func(const CallableCustom *a, const CallableCustom *b);

  [[nodiscard]] CompareEqualFunc get_compare_equal_func() const override;

  static bool compare_less_func(const CallableCustom *a, const CallableCustom *b);

  [[nodiscard]] CompareLessFunc get_compare_less_func() const override;

  [[nodiscard]] bool is_valid() const override;

  [[nodiscard]] ObjectID get_object() const override;

  void call(const Variant **args,
      int arg_count,
      Variant &return_value,
      GDExtensionCallError &return_call_error) const override;

private:
  s7_scheme *sc;
  s7_protected_ptr f;
  bool discard_return_value;
};
} //namespace godot
#endif //SX7_CALLABLE_H
