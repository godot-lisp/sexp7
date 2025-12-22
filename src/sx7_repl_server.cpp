#include "sx7_repl_server.hpp"
#include "repl/mediator.hpp"
#include "sx7.hpp"
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <limits>

using namespace godot;
using gd = UtilityFunctions;

void SX7ReplServer::publish_node(const SX7 *node) {
  if (thread.is_null()) {
    return;
  }
  auto node_name = node->get_path().slice(-2).get_concatenated_names();
  message_queue.push(
      ReplMessage::publish_node(
          std::move(node_name),
          node->get_instance_id()));
}

void SX7ReplServer::unpublish_node(const SX7 *node) {
  if (thread.is_null()) {
    return;
  }
  message_queue.push(
      ReplMessage::unpublish_node(
          node->get_instance_id()));
}

void SX7ReplServer::reply(String result, uint64_t connection_id) {
  if (thread.is_null()) {
    return;
  }
  message_queue.push(
      ReplMessage::eval_response(
          connection_id,
          std::move(result)));
}

void SX7ReplServer::server_loop(int tcp_port, const String &tcp_bind_address) {
  Ref<TCPServer> tcp_server;
  tcp_server.instantiate();

  auto error = tcp_server->listen(tcp_port, tcp_bind_address);
  ERR_FAIL_COND_MSG(
      error != OK,
      ("Failed to start sx7 repl server: " + gd::error_string(error)));

  gd::print("SX7 repl server listening on local port ", tcp_server->get_local_port());

  auto mediator = ReplMediator(tcp_server, Callable::create(this, "reply"));
  while (!exit_thread) {
    if (!mediator.mediate(message_queue)) {
      OS::get_singleton()->delay_msec(50);
    }
  }
  tcp_server->stop();
}

using tcp_port_t = uint16_t;

std::optional<std::pair<tcp_port_t, String>> parse_repl_args() {
  // TODO: accept --s7-tcp-address=<address>
  String tcp_bind_address = "127.0.0.1";
  String tcp_port_option = "--s7-tcp-port";
  for (const auto &arg : OS::get_singleton()->get_cmdline_args()) {
    if (arg.begins_with(tcp_port_option)) {
      auto parts = arg.split("=");
      auto tcp_port = parts.size() > 1 ? parts[1].to_int() : 0;
      auto max_tcp_port = std::numeric_limits<tcp_port_t>::max();
      if (tcp_port < 0 || tcp_port >= max_tcp_port) {
        gd::printerr(tcp_port_option, " argument must be a value between 0 and ", max_tcp_port, ".");
      }
      return std::make_pair(static_cast<tcp_port_t>(tcp_port), std::move(tcp_bind_address));
    }
  }
  return std::nullopt;
}

Error SX7ReplServer::start() {
  ERR_FAIL_COND_V_MSG(thread.is_valid(), ERR_BUG, "SX7 repl server can only be started once!");

  auto repl_args = parse_repl_args();
  if (!repl_args) {
    return Error::OK;
  }

  const auto &[tcp_port, tcp_bind_address] = *repl_args;

  exit_thread = false;
  thread.instantiate();
  return thread->start(
      Callable::create(this, "server_loop").bind(tcp_port, tcp_bind_address),
      Thread::PRIORITY_LOW);
}

void SX7ReplServer::stop() {
  if (thread.is_null()) {
    return;
  }

  exit_thread = true;
  thread->wait_to_finish();

  thread.unref();
}

SX7ReplServer *SX7ReplServer::singleton = nullptr;

SX7ReplServer *SX7ReplServer::get_singleton() {
  return singleton;
}

SX7ReplServer::SX7ReplServer() {
  singleton = this;
}

SX7ReplServer::~SX7ReplServer() {
  singleton = nullptr;
}

void SX7ReplServer::_bind_methods() {
  ClassDB::bind_method(D_METHOD("server_loop"), &SX7ReplServer::server_loop);
  ClassDB::bind_method(D_METHOD("reply"), &SX7ReplServer::reply);
}
