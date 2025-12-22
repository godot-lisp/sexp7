#include "mediator.hpp"
#include "../sx7.hpp"
#include "debug.hpp"
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

ReplMediator::ReplMediator(Ref<TCPServer> server, Callable reply) :
    server(server), eval_async_continuation(reply) {
  node_registry = std::make_shared<ReplNodeRegistry>();
}

struct ReplMessageHandler {
  ReplMediator *mediator;

  void operator()(const ReplMessage::PublishNode &m) {
    mediator->node_registry->register_node(m.node_id, std::move(m.node_name));
  }
  void operator()(const ReplMessage::UnpublishNode &m) {
    mediator->node_registry->unregister_node(m.node_id);
  }
  void operator()(const ReplMessage::EvalResponse &m) {
    for (auto &[connection, context] : mediator->connections) {
      if (context.connection_id == m.connection_id) {
        connection.on_eval_async_result(m.result);
        break;
      }
    }
  }
};

bool ReplMediator::ReplConnectionContext::eval_async(const String &code, uint64_t scheme_node_id) {
  auto scheme_node = ObjectDB::get_instance(scheme_node_id);
  if (!scheme_node) {
    return false;
  }
  scheme_node->call_deferred(
      "eval_async",
      code,
      mediator->eval_async_continuation.bind(connection_id));
  return true;
}

bool ReplMediator::mediate(MessageQueue &queue) {
  int interactions = 0;
  auto message = queue.pop();
  if (message) {
    std::visit(ReplMessageHandler{ this }, message->payload);
    interactions++;
  }

  if (server->is_connection_available()) {
    auto connection = ReplConnection(server->take_connection(), node_registry);
    gd::print("Scheme repl client connected.");
    connection.send_prompt();
    connections.emplace_back(std::move(connection), ReplConnectionContext(++next_id, this));
    interactions++;
  }

  for (auto it = connections.begin(); it != connections.end();) {
    auto &[connection, context] = *it;
    auto status = connection.process_with(context);
    if (status == ReplConnection::DISCONNECTED) {
      it = connections.erase(it);
      gd::print("Scheme repl client disconnected.");
    } else {
      it++;
      interactions += (status != ReplConnection::IDLE) ? 1 : 0;
    }
  }
  return interactions > 0;
}
