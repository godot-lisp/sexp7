#include "sx7.hpp"
#include "ffi.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include "sx7_repl_server.hpp"

using namespace godot;

SX7::SX7() {
  define_variant_ffi(sx7);
  auto node = make_variant_object(sx7.get(), this);
  sx7.define_constant_with_documentation("*node*", node, "this Godot node");
}

SX7::~SX7() { _process_symbol = nullptr; }

void SX7::_ready() {
  load_prelude();
  load_script();
  sx7.call_optional("_ready");
}

void SX7::_process(double delta) {
  if (_process_symbol) {
    sx7.call(_process_symbol.get(), delta);
  }
}
void SX7::_input(const Ref<InputEvent> &p_event) {
    auto sc = sx7.get();
    auto func = s7_name_to_value(sc, "input");
    if (s7_is_procedure(func)) {
        auto event_obj = variant_to_scheme(sc, p_event);
        s7_call(sc, func, s7_list(sc, 1, event_obj));
    }
}
void SX7::_enter_tree() {
  // TODO: move initialization here
  auto server = SX7ReplServer::get_singleton();
  if (server) server->publish_node(this);

  Control::_enter_tree();
}

void SX7::_exit_tree() {
  auto server = SX7ReplServer::get_singleton();
  if (server) server->unpublish_node(this);

  if (_process_symbol) {
    auto _ = sx7.call_optional("_exit_tree");
  }
  Control::_exit_tree();
}

void SX7::define(
    const godot::String &name,
    const godot::Variant &value,
    const String &help) const {
  sx7.define(name.utf8(), variant_to_scheme(sx7.get(), value), help.utf8());
}

void SX7::set_scheme_script(const Ref<godot::SX7Script> &p_scheme_script) {
  scheme_script = p_scheme_script;
  if (is_node_ready()) {
    load_script();
  }
}

void SX7::load_prelude() {
  for (int i = 0; i < prelude.size(); ++i) {
    auto script = Object::cast_to<SX7Script>(prelude[i]);
    DEV_ASSERT(script != nullptr);
    load(script);
  }
}

void SX7::load_script() {
  if (scheme_script.is_null()) {
    _process_symbol = nullptr;
    return;
  }

  load(scheme_script.ptr());
  _process_symbol = sx7.make_symbol("_process");
}

void SX7::load(const godot::SX7Script *script) const {
  load_string(script->get_code());
}

void SX7::load_string(const String &code) const {
  sx7.load_string(code);
}

Variant SX7::eval(const String &code) {
  return scheme_to_variant(sx7.get(), sx7.eval(code));
}

void SX7::eval_async(const String &code, const Callable &continuation) {
  auto result = scheme_object_to_godot_string(sx7.get(), sx7.eval(code));
  continuation.call_deferred(result);
}

s7_pointer array_to_list(s7_scheme *sc, const Array &array) {
  auto list = s7_nil(sc);
  auto arg_count = static_cast<int>(array.size());
  while (arg_count > 0) {
    auto arg = variant_to_scheme(sc, array[--arg_count]);
    list = s7_cons(sc, arg, list);
  }
  return list;
}

Variant SX7::apply(const String &symbol, const Array &args) const {
  auto sc = sx7.get();
  auto func = s7_name_to_value(sc, symbol.utf8().ptr());
  auto scheme_args = array_to_list(sc, args);
  return scheme_to_variant(sc,
      s7_call_with_location(sc, func, scheme_args, __func__, __FILE__, __LINE__));
}

void SX7::_bind_methods() {
  ClassDB::bind_method(D_METHOD("set_prelude", "p_prelude"), &SX7::set_prelude);
  ClassDB::bind_method(D_METHOD("get_prelude"), &SX7::get_prelude);
  ClassDB::add_property("SX7",
      PropertyInfo(Variant::ARRAY,
          "prelude",
          PROPERTY_HINT_TYPE_STRING,
          vformat("%d/%d:SX7Script", Variant::OBJECT, PROPERTY_HINT_RESOURCE_TYPE)),
      "set_prelude",
      "get_prelude");

  ClassDB::bind_method(
      D_METHOD("set_scheme_script", "p_scheme_script"),
      &SX7::set_scheme_script);
  ClassDB::bind_method(D_METHOD("get_scheme_script"), &SX7::get_scheme_script);
  ClassDB::add_property("SX7",
      PropertyInfo(
          Variant::OBJECT,
          "scheme_script",
          PROPERTY_HINT_RESOURCE_TYPE,
          "SX7Script"),
      "set_scheme_script",
      "get_scheme_script");

  ClassDB::bind_method(
      D_METHOD("define", "p_name", "p_value", "p_help"),
      &SX7::define,
      DEFVAL(""));
  ClassDB::bind_method(D_METHOD("eval", "p_code"), &SX7::eval);
  ClassDB::bind_method(D_METHOD("eval_async", "p_code", "p_continuation"), &SX7::eval_async);
  ClassDB::bind_method(D_METHOD("apply", "p_symbol", "p_args"),
      &SX7::apply,
      DEFVAL(Array()));
  ClassDB::bind_method(D_METHOD("load", "p_scheme_script"), &SX7::load);
  ClassDB::bind_method(D_METHOD("load_string", "p_code"), &SX7::load_string);
}
