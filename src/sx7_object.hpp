
#ifndef SX7_OBJECT_H
#define SX7_OBJECT_H

#include "s7.hpp"

#include <godot_cpp/classes/ref_counted.hpp>

namespace godot {
class SX7Object : public RefCounted {
  GDCLASS(SX7Object, RefCounted)

public:
  SX7Object() :
      sc(nullptr), scheme_ptr(nullptr) {}
  SX7Object(s7_scheme *sc, s7_pointer shared) :
      sc(sc), scheme_ptr(s7_gc_protected(sc, shared)) {}

  bool belongs_to(const s7_scheme *scheme) const {
    return sc == scheme;
  }

  [[nodiscard]] s7_pointer get_scheme_ptr() const {
    return scheme_ptr.get();
  }

protected:
  static void _bind_methods();

private:
  const s7_scheme *sc;
  s7_protected_ptr scheme_ptr;
};
} //namespace godot

#endif //SX7_OBJECT_H
