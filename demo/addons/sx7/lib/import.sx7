(provide 'import)

(require 'array)

(define (snake-case->lisp-case s)
  (list->string
   (map
    (lambda (c)
      (if (char=? #\_ c) #\- c))
    s)))

(define (adjust-name-case name lisp-case)
  (if lisp-case (snake-case->lisp-case name) name))

(define (string->symbol-with-prefix prefix string lisp-case)
  (string->symbol (string-append (symbol->string prefix) "/" (adjust-name-case string lisp-case))))

(define (symbol->Variant symbol)
  (Variant (symbol->string symbol)))

(define (Variant->symbol var lisp-case)
  (string->symbol (adjust-name-case (Variant->string var) lisp-case)))

(define (public-instance-method-name? name)
  (not (char=? #\_ (name 0))))

(define (import-method class method-info prefix lisp-case)
  (let ((name (Variant->string (method-info 'name))))
    (when (public-instance-method-name? name)
      (let* ((args (method-info 'args))
             (ps (Array->list args (lambda (arg) (Variant->symbol (arg 'name) lisp-case))))
             (ps-doc
               (format #f "(~{~A~^, ~})"
                 (Array->list args
                   (lambda (arg)
                     (format #f "~A: ~A" (Variant->string (arg 'name)) (VariantType->string (arg 'type)))))))
             (doc
              (string-append
               (let ((rt (method-info 'return 'type)))
                 (if (= 0 rt) "void" (VariantType->string rt)))
               " " (symbol->string class) "." name ps-doc)))

        `(define (,(string->symbol-with-prefix prefix name lisp-case) self ,@ps)
           ,doc
           (! self ',(string->symbol name) ,@ps))))))

(define (import-integer-constants-of class as include-inherited lisp-case)
  (let ((class-name (symbol->Variant class))
        (no-inheritance (not include-inherited)))
    `(begin
      ,@(Array->list
        (! (class-db) 'class_get_integer_constant_list class-name no-inheritance)
        (lambda (c)
          (let ((value (! (class-db) 'class_get_integer_constant class-name c)))
            `(define-constant ,(string->symbol-with-prefix as (Variant->string c) lisp-case) ,value)))))))

(define-macro* (import-class class (as #f) (include-inherited #f) (lisp-case #t) (only ()))
  (let* ((class-name (symbol->Variant class))
         (no-inheritance (not include-inherited))
         (ms (! (class-db) 'class_get_method_list class-name no-inheritance))
         (as (or as class)))
    `(begin
       ,(import-integer-constants-of class as include-inherited lisp-case)
       ,@(Array->list ms (lambda (m) (import-method class m as lisp-case)))
       #t)))
