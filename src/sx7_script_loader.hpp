#ifndef SEXP7_SX7_SCRIPT_LOADER_H
#define SEXP7_SX7_SCRIPT_LOADER_H

#include <godot_cpp/classes/resource_format_loader.hpp>

namespace godot {
class SX7ScriptLoader : public ResourceFormatLoader {
  GDCLASS(SX7ScriptLoader, ResourceFormatLoader)

public:
  SX7ScriptLoader();
  ~SX7ScriptLoader();

  PackedStringArray _get_recognized_extensions() const override;
  bool _handles_type(const StringName &type) const override;
  Variant _load(const String &path,
      const String &original_path,
      bool use_sub_threads,
      int32_t cache_mode) const override;
  String _get_resource_type(const String &path) const override;
  //  bool _recognize_path(const String &path, const StringName &type) const override;
  //  String _get_resource_script_class(const String &path) const override;
  //  int64_t _get_resource_uid(const String &path) const override;
  //  PackedStringArray _get_dependencies(const String &path, bool add_types) const
  //  override; Error _rename_dependencies(const String &path, const Dictionary &renames)
  //  const override; bool _exists(const String &path) const override; PackedStringArray
  //  _get_classes_used(const String &path) const override;

protected:
  static void _bind_methods();

private:
};

} // namespace godot

#endif //SEXP7_SX7_SCRIPT_LOADER_H
