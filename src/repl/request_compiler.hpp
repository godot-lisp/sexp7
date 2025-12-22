#ifndef SEXP7_REPL_REQUEST_COMPILER_HPP
#define SEXP7_REPL_REQUEST_COMPILER_HPP

#include "../s7.hpp"
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/string.hpp>

using error_output_and_response = std::pair<godot::String, godot::String>;

class ReplRequestCompiler {
public:
  ReplRequestCompiler();
  ~ReplRequestCompiler();

public:
  error_output_and_response compile_request(const godot::PackedByteArray &request);
  godot::String eval(const godot::String& compiled_request);

private:
  s7_protected_ptr compile_geiser_request;
  s7 sx7;
};
#endif
