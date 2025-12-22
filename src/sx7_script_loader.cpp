#include "sx7_script_loader.hpp"
#include "sx7_script.hpp"
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void SX7ScriptLoader::_bind_methods() {}

SX7ScriptLoader::SX7ScriptLoader() {}

SX7ScriptLoader::~SX7ScriptLoader() {}

PackedStringArray SX7ScriptLoader::_get_recognized_extensions() const {
  auto extensions = PackedStringArray();
  extensions.append("sx7");
  return extensions;
}

bool SX7ScriptLoader::_handles_type(const StringName &type) const {
  return type == String("SX7Script");
}

Variant SX7ScriptLoader::_load(
    const String &path,
    const String &original_path,
    bool use_sub_threads,
    int32_t cache_mode) const {
  auto code = FileAccess::get_file_as_string(path);
  return memnew(SX7Script(code));
}

String SX7ScriptLoader::_get_resource_type(const String &p_path) const {
  if (p_path.get_extension().to_lower() == "sx7") {
    return "SX7Script";
  }
  return "";
}
