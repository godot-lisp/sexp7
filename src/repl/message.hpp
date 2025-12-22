#ifndef SEXP7_REPL_MESSAGE_HPP
#define SEXP7_REPL_MESSAGE_HPP

#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <variant>

struct ReplMessage {
  static ReplMessage publish_node(godot::StringName &&node_name, uint64_t node_id) {
    return ReplMessage{ PublishNode{ std::move(node_name), node_id } };
  }

  static ReplMessage unpublish_node(uint64_t node_id) {
    return ReplMessage{ UnpublishNode{ node_id } };
  }

  static ReplMessage eval_response(uint64_t connection_id, godot::String &&result) {
    return ReplMessage{ EvalResponse{ connection_id, std::move(result) } };
  }

  struct PublishNode {
    godot::StringName node_name;
    uint64_t node_id;
  };

  struct UnpublishNode {
    uint64_t node_id;
  };

  struct EvalResponse {
    uint64_t connection_id;
    godot::String result;
  };

  using Payload = std::variant<PublishNode, UnpublishNode, EvalResponse>;
  Payload payload;
};

#endif
