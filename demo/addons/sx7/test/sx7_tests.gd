extends Node

func make_node(name: String):
  var n = Node.new()
  n.name = name
  return n

func define(name: String, value: Variant):
  $SX7.define(name, value)

func show_value_with_type(r):
  return "%s(%s)" % [type_string(typeof(r)), r]

func eval(code: String):
  var r = $SX7.eval(code)
  print(code, "=>", show_value_with_type(r))
  return r

func apply(symbol: String, args: Array):
  var r = $SX7.apply(symbol, args)
  print("(apply ", symbol, " ", args, ")=>", show_value_with_type(r))
  return r

func can_exchange_primitive_values():
  eval("(format #f \"Hello from Scheme!~%\")")

  define("an-integer", 41)
  eval("an-integer")
  eval("(+ 1 an-integer)")

  define("a-float", 41.0)
  eval("a-float")
  eval("(+ 1 a-float)")

  define("a-true", true)
  define("a-false", false)
  eval("(if a-false #t #f)")
  eval("(if a-true #t #f)")
  eval("(if (not a-false) #f #t)")
  eval("(if (not a-true) #f #t)")

func can_connect_signal_to_symbol():
  # given: a signal connected to a symbol
  $SX7.eval("""
    (begin
      (define (handler n) (print "v1:" (n 'name)))
      (connect! *node* 'child_entered_tree 'handler))
  """)
  eval("(connected? *node* 'child_entered_tree 'handler)")
  $SX7.add_child(make_node("s1"))
  # when: procedure is update
  $SX7.eval("""
    (define (handler n) (print "v2:" (n 'name)))
  """)
  # then: signal should trigger new procedure
  $SX7.add_child(make_node("s2"))
  # when: symbol is disconnected from signal
  $SX7.eval("""
    (disconnect! *node* 'child_entered_tree 'handler)
  """)
  eval("(connected? *node* 'child_entered_tree 'handler)")
  # then: signal no longer triggers the procedure
  $SX7.add_child(make_node("s3"))

func can_connect_signal_to_procedure():
  # given: a signal connected to a procedure
  $SX7.eval("""
    (begin
      (define (handler n) (print "v1:" (n 'name)))
      (connect! *node* 'child_entered_tree handler))
  """)
  eval("(connected? *node* 'child_entered_tree handler)")
  $SX7.add_child(make_node("p1"))
  # when: procedure is update
  $SX7.eval("""
    (define original-handler handler)
    (define (handler n) (print "v2:" (n 'name)))
  """)
  # then: signal should trigger OLD procedure
  $SX7.add_child(make_node("p2"))
  # when: symbol is disconnected from signal
  $SX7.eval("""
    (disconnect! *node* 'child_entered_tree original-handler)
  """)
  eval("(connected? *node* 'child_entered_tree original-handler)")
  # then: signal no longer triggers the procedure
  $SX7.add_child(make_node("p3"))

func can_compare_variants():
  define("v1", [1, 2, 3])
  define("v2", [1, 2, 3, 4])
  define("v3", [1, 2, 3])
  define("v4", "foo")
  define("v5", "bar")
  define("v6", "foo")
  eval("(equal? v1 v2)")
  eval("(equal? v1 v3)")
  eval("(equal? v4 v5)")
  eval("(equal? v4 v6)")

func can_query_variant_type():
  # int, float and boolean values are never stored as variants
  define("i1", 42)
  define("f1", 42.0)
  define("b1", true)
  define("b2", false)
  eval("(or (Variant? i1) (Variant? f1) (Variant? b1) (Variant? b2))")

  # everything else, including Strings are Variants
  define("v1", [])
  eval("(VariantType->string (Variant? v1))")
  define("v2", {"foo": "bar"})
  eval("(VariantType->string (Variant? v2))")
  define("v3", "Strings are not automatically converted to Scheme")
  eval("(VariantType->string (Variant? v3))")

func can_create_arrays():
  eval("(Array)")
  eval("(Array 1 2.0 \"three\" #t #f)")

func can_create_vectors():
  eval("(Vector2 1.0 2.0)")
  eval("(Vector2 1 2)")
  eval("(Vector2i 1 2)")

func can_create_rects():
  eval("(Rect2 1.0 2.0 3.0 4.0)")
  eval("(Rect2 1 2 3 4)")
  eval("(Rect2i 1 2 3 4)")

func can_map_arrays():
  define("a1", [1, 2, 3])
  eval("(! a1 'map (lambda (x) (* 2 x)))")
  eval("(a1 `(map ,(lambda (x) (* 3 x))))")
  eval("(define (f x) (* 4 x))")
  eval("a1")

  # Callable discards return values by default
  eval("(! a1 'map (Callable 'f))")
  eval("(! a1 'map (Callable 'f #f))")
  eval("(! a1 'map (Callable 'f #t))")

func can_import_classes():
  eval("""
    (begin
      (require 'import)
      (import-class Performance :as p)
      (help 'p/get-monitor))
  """)

func can_import_singletons():
  eval("""
    (begin
      (require 'import)
      (import-singleton Performance :as p)
      (help 'p/get-monitor))
  """)

func can_create_dictionaries():
  eval("(let ((d (Dictionary))) (set! (d 0) (Color \"red\")) d)")

func can_iterate_on_variants():
  eval("""
    (define (test-for-each-on xs)
      (call-with-output-string
        (lambda (p)
          (for-each
            (lambda (x) (format p "(~A)" x))
            xs))))
  """)
  define("array", [1, 2, 3])
  eval("(test-for-each-on array)")
  define("dictionary", {"foo": "bar"})
  eval("(test-for-each-on dictionary)")
  define("string", "Scheme")
  eval("(test-for-each-on string)")

func can_use_tree_sitter_api():
  eval("""
    (call-with-output-string (lambda (p)

      (require 'tree-sitter)

      (define (print-node node depth)
        (format p "~A~A..~A:~A:~A~%"
          (if (> depth 0) (make-string (* depth 2) #\\ ) "\\n")
          (ts-node-start-point node)
          (ts-node-end-point node)
          (ts-node-symbol node)
          (ts-node-type node)))

      (define (print-subtree node depth)
        (print-node node depth)
        (for-each
          (lambda (child) (print-subtree child (+ 1 depth)))
          node))

      (let* ((tree (ts-parser-parse-string (ts-parser-new) "(Scheme\\nrulez!)"))
             (root (ts-tree-root-node tree)))
        (print-subtree root 0)
        (let ((cursor (ts-tree-cursor-new root)))
          (format p "first-child-for-point (1 . 0) => ~A" (ts-tree-cursor-goto-first-child-for-point cursor '(1 . 0)))
          (print-node (ts-tree-cursor-current-node cursor) 0)))))
  """)

func roundtrip(expr: String, type_test: String):
  var v = eval(expr)
  define("roundtrip", v)
  eval("(%s roundtrip)" % type_test)

func char_becomes_int():
  roundtrip("#\\c", "char?") # false
  eval("(integer? roundtrip)")

func symbol_roundtrips_as_StringName():
  roundtrip(":a-keyword-symbol", "symbol?")

func producedure_roundtrips_as_Callable():
  roundtrip("(lambda (x) (* x 2))", "procedure?")
  eval("(roundtrip 21)")

func can_pass_arbitrary_scheme_objects_back_n_forth():
  roundtrip("#t", "boolean?")
  roundtrip("42", "integer?")
  roundtrip("42.0", "real?")
  roundtrip("()", "list?")
  roundtrip("'(1 . 2)", "pair?")
  roundtrip("'(1 2 3)", "pair?")
  roundtrip("#(1 2 3)", "vector?")
  roundtrip("#<undefined>", "undefined?")

func can_apply_functions():
  eval("(define* (scheme-function (a #f) (b #t) (c 42)) (object->string (list a b c)))")

  # apply with no arguments first
  var r = $SX7.apply("scheme-function")
  print("(apply scheme-function)=>", show_value_with_type(r))

  apply("scheme-function", [1])
  apply("scheme-function", [1, 2])
  apply("scheme-function", [1, 2, 3])
  apply("scheme-function", [1, 2, 3, 4])
  apply("scheme-function", [1, &":b", 2])
  apply("scheme-function", [1, &":b", 2, &":c", 33])
  apply("non-existing-function", [])

func _ready():
  can_exchange_primitive_values()
  can_connect_signal_to_symbol()
  can_connect_signal_to_procedure()
  can_compare_variants()
  can_query_variant_type()
  can_create_arrays()
  can_map_arrays()
  can_create_vectors()
  can_create_rects()
  can_import_classes()
  #can_import_singletons()
  can_create_dictionaries()
  can_iterate_on_variants()
  #can_use_tree_sitter_api()
  char_becomes_int()
  symbol_roundtrips_as_StringName()
  producedure_roundtrips_as_Callable()
  can_pass_arbitrary_scheme_objects_back_n_forth()
  can_apply_functions()
  get_tree().quit()
