#include "request_compiler.hpp"
#include "../ffi.hpp"
#include "debug.hpp"
#include "gen/sx7_scheme_repl_string.hpp"
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

template <typename T, typename Function>
std::pair<String, T> eval_capturing_error_output(s7_scheme *sc, Function f) {
  auto string_port = s7_open_output_string(sc);
  auto previous_error_port = s7_set_current_error_port(sc, string_port);
  auto result = f(sc);
  s7_set_current_error_port(sc, previous_error_port);
  auto output = s7_output_string(sc, string_port);
  return std::make_pair(scheme_string_to_godot_string(output), result);
}

ReplRequestCompiler::ReplRequestCompiler() {
  compile_geiser_request = sx7.make_symbol("compile-geiser-request");
  auto [output, _] = eval_capturing_error_output<nullptr_t>(sx7.get(), [](auto sc) {
    DEBUG_REPL(sx7_scheme_repl_string);
    s7_load_c_string(sc,
        sx7_scheme_repl_string,
        strlen(sx7_scheme_repl_string));
    return nullptr;
  });
  if (!output.is_empty()) {
    gd::printerr(output);
  }
}

ReplRequestCompiler::~ReplRequestCompiler() {
  compile_geiser_request = nullptr;
}

String ReplRequestCompiler::eval(const String &compiled_request) {
  auto sc = sx7.get();
  auto res = sx7.eval(compiled_request);
  return scheme_object_to_godot_string(sc, res);
}

error_output_and_response ReplRequestCompiler::compile_request(const PackedByteArray &request) {
  auto compile_geiser_request = sx7.value_of(this->compile_geiser_request);
  if (!s7_is_procedure(compile_geiser_request)) {
    return std::make_pair("repl script is missing a compile-geiser-request function!", "");
  }

  auto sc = sx7.get();
  auto [output, compiled_request] =
      eval_capturing_error_output<s7_pointer>(sc,
          [compile_geiser_request, &request](auto sc) {
            auto args = s7_cons(sc,
                s7_make_string_wrapper_with_length(
                    sc,
                    reinterpret_cast<const char *>(request.ptr()),
                    static_cast<s7_int>(request.size())),
                s7_nil(sc));
            return s7_call_with_location(
                sc,
                compile_geiser_request,
                args,
                __func__,
                __FILE__,
                __LINE__);
          });

  if (!s7_is_string(compiled_request)) {
    DEBUG_REPL(scheme_object_to_godot_string(sc, compiled_request));
    return std::make_pair(output, scheme_object_to_godot_string(sc, compiled_request));
  }

  auto code_string = scheme_string_to_godot_string(compiled_request);
  DEBUG_REPL("```\n", code_string, "\n```");
  return std::make_pair(output, code_string);
}
