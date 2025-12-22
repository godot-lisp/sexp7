#ifndef SEXP7_SX7_REPL_SERVER_H
#define SEXP7_SX7_REPL_SERVER_H

#include "repl/mediator.hpp"
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/thread.hpp>

namespace godot {

class SX7;

class SX7ReplServer : public Object {
  GDCLASS(SX7ReplServer, Object);

public: // public API
  static SX7ReplServer *get_singleton();
  void publish_node(const SX7 *node);
  void unpublish_node(const SX7 *node);
  void reply(String result, uint64_t connection_id);

private:
  static SX7ReplServer *singleton;
  bool exit_thread;
  Ref<Thread> thread;
  ReplMediator::MessageQueue message_queue;

public: // extension initialization API
  SX7ReplServer();
  ~SX7ReplServer();
  Error start();
  void stop();

protected:
  static void _bind_methods();
  void server_loop(int tcp_port, const String &tcp_bind_address);
};
} //namespace godot
#endif //SEXP7_SX7_REPL_SERVER_H
