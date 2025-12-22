#ifndef SEXP7_SX7_SCRIPT_H
#define SEXP7_SX7_SCRIPT_H

#include <godot_cpp/classes/resource.hpp>

namespace godot {
class SX7Script : public Resource {
  GDCLASS(SX7Script, Resource)

public:
  SX7Script();
  SX7Script(const String &code) :
      code(code) {}
  ~SX7Script();

  const String &get_code() const { return code; }

protected:
  static void _bind_methods();

private:
  String code;
};

} // namespace godot

#endif //SEXP7_SX7_SCRIPT_H
