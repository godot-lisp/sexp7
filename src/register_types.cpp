#include "register_types.h"
#include "sx7.hpp"
#include "sx7_object.hpp"
#include "sx7_repl_server.hpp"
#include "sx7_script.hpp"
#include "sx7_script_loader.hpp"
#include <gdextension_interface.h>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

static Ref<SX7ScriptLoader> script_loader;

void initialize_scene_types() {
  GDREGISTER_CLASS(SX7Script);
  GDREGISTER_CLASS(SX7ScriptLoader);
  GDREGISTER_CLASS(SX7);
  GDREGISTER_CLASS(SX7Object);

  script_loader.instantiate();
  ResourceLoader::get_singleton()->add_resource_format_loader(script_loader);
}

void uninitialize_scene_types() {
  ResourceLoader::get_singleton()->remove_resource_format_loader(script_loader);
  script_loader.unref();
}

static SX7ReplServer *repl_server = nullptr;

void initialize_server_types() {
  GDREGISTER_CLASS(SX7ReplServer);
  repl_server = memnew(SX7ReplServer);
  repl_server->start();
}

void uninitialize_server_types() {
  repl_server->stop();
  memdelete(repl_server);
}

void initialize_gdextension_types(ModuleInitializationLevel p_level) {
  switch (p_level) {
    case MODULE_INITIALIZATION_LEVEL_SCENE:
      initialize_scene_types();
      break;
    case MODULE_INITIALIZATION_LEVEL_EDITOR:
      initialize_server_types();
      break;
    default:
      break;
  }
}

void uninitialize_gdextension_types(ModuleInitializationLevel p_level) {
  switch (p_level) {
    case MODULE_INITIALIZATION_LEVEL_SCENE:
      uninitialize_scene_types();
      break;
    case MODULE_INITIALIZATION_LEVEL_EDITOR:
      uninitialize_server_types();
      break;
    default:
      break;
  }
}

extern "C" {
GDExtensionBool GDE_EXPORT sexp7_library_init(
    GDExtensionInterfaceGetProcAddress p_get_proc_address,
    GDExtensionClassLibraryPtr p_library,
    GDExtensionInitialization *r_initialization) {
  GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);
  init_obj.register_initializer(initialize_gdextension_types);
  init_obj.register_terminator(uninitialize_gdextension_types);
  init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

  return init_obj.init();
}
}
