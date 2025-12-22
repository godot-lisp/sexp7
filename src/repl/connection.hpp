#ifndef SEXP7_REPL_REQUEST_CONNECTION_HPP
#define SEXP7_REPL_REQUEST_CONNECTION_HPP

#include "node_registry.hpp"
#include "request_compiler.hpp"
#include <godot_cpp/classes/stream_peer_tcp.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <memory>

class ReplConnection {
public:
  struct Context {
    virtual ReplRequestCompiler &compiler() = 0;
    virtual bool eval_async(const godot::String &compiled_request, uint64_t node_instance_id) = 0;
  };

  enum Status {
    IDLE,
    TRANSMITTING,
    DISCONNECTED
  };

  ReplConnection(
      godot::Ref<godot::StreamPeerTCP> tcp_stream,
      std::shared_ptr<ReplNodeRegistry> node_registry) :
      tcp_stream(tcp_stream),
      node_registry(node_registry),
      target_node(node_registry->get_most_recent()) {}

public:
  void send_prompt();
  Status process_with(Context &context);
  void on_eval_async_result(const godot::String &s);
  void disconnect();

private:
  godot::String get_prompt();
  void send_output(const godot::String &s);
  void send(const godot::String &s);
  void send(const char *p, size_t count);
  void send(char c);
  bool process_buffer_with(Context &context);
  void process_eval_request_with(Context &context);
  void send_available_nodes();
  void enter_selected_node(const godot::String &node_name);

private:
  godot::Ref<godot::StreamPeerTCP> tcp_stream;
  godot::PackedByteArray buffer;
  std::shared_ptr<ReplNodeRegistry> node_registry;
  std::optional<ReplNodeRegistry::NodeRecord> target_node;
};
#endif
