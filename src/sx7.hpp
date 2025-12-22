#ifndef GDS7_H
#define GDS7_H

#include "s7.hpp"
#include "sx7_script.hpp"
#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/variant/typed_array.hpp>

namespace godot {
class SX7 : public Control {
  GDCLASS(SX7, Control)

public:
  SX7();
  ~SX7() override;

  void _ready() override;
  void _process(double delta) override;
  void _enter_tree() override;
  void _exit_tree() override;
  void _input(const Ref<InputEvent> &p_event) override;

  void define(const String &name, const Variant &value, const String &help = "") const;
  void load(const SX7Script *script) const;
  void load_string(const String &code) const;
  Variant eval(const String &code);
  /**
   * Process an async evaluation request from repl and calls [continuation] with the result.
   */
  void eval_async(const String &code, const Callable &continuation);
  Variant apply(const String &symbol, const Array &args) const;
  void set_prelude(const TypedArray<SX7Script> &p_prelude) { prelude = p_prelude; }
  [[nodiscard]] TypedArray<SX7Script> get_prelude() const { return prelude; }
  void set_scheme_script(const Ref<SX7Script> &p_scheme_script);
  [[nodiscard]] Ref<SX7Script> get_scheme_script() const { return scheme_script; };

  [[nodiscard]] const s7 &get_s7() const { return sx7; }

protected:
  static void _bind_methods();

private:
  void load_prelude();
  void load_script();

private:
  TypedArray<SX7Script> prelude;
  Ref<SX7Script> scheme_script;
  s7_protected_ptr _process_symbol;
  s7 sx7;
};

} // namespace godot

#endif
