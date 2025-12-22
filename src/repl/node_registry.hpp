#ifndef SEXP7_REPL_NODE_REGISTRY_HPP
#define SEXP7_REPL_NODE_REGISTRY_HPP

#include <godot_cpp/variant/string.hpp>
#include <optional>
#include <vector>

class ReplNodeRegistry {
public:
  struct NodeRecord {
    uint64_t node_id;
    godot::String node_name;
  };

  std::optional<NodeRecord> get_most_recent();
  std::vector<godot::String> get_available_node_names();
  std::optional<NodeRecord> find_node_by_name(const godot::String& node_name);
  void register_node(uint64_t node_id, godot::String &&node_name);
  void unregister_node(uint64_t node_id);

private:
  std::vector<NodeRecord> nodes;
};
#endif
