#ifndef SEXP7_REPL_REQUEST_MEDIATOR_HPP
#define SEXP7_REPL_REQUEST_MEDIATOR_HPP

#include "connection.hpp"
#include "message.hpp"
#include "node_registry.hpp"
#include "request_compiler.hpp"
#include "thread_safe_queue.hpp"
#include <godot_cpp/classes/tcp_server.hpp>
#include <vector>

class ReplMediator {
public:
  using MessageQueue = ThreadSafeQueue<ReplMessage>;

  ReplMediator(godot::Ref<godot::TCPServer> server, godot::Callable eval_async_continuation);

public:
  /**
   * Process all pending interactions.
   * @return true when at least one interaction was processed, false otherwise.
   */
  bool mediate(MessageQueue &queue);

private:
  friend struct ReplMessageHandler;

  struct ReplConnectionContext : ReplConnection::Context {
    uint64_t connection_id;
    ReplMediator *mediator;
    ReplConnectionContext(uint64_t id, ReplMediator *mediator) :
        connection_id(id), mediator(mediator) {}
    ReplRequestCompiler &compiler() override { return mediator->request_compiler; };
    bool eval_async(const godot::String &code, uint64_t scheme_node_id) override;
  };

  friend struct ReplMediator::ReplConnectionContext;

private:
  godot::Ref<godot::TCPServer> server;
  godot::Callable eval_async_continuation;
  std::shared_ptr<ReplNodeRegistry> node_registry;
  std::vector<std::pair<ReplConnection, ReplConnectionContext>> connections;
  ReplRequestCompiler request_compiler;
  uint64_t next_id = 0;
};
#endif
