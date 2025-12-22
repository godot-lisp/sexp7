(provide 'array)

(define* (Array->vector array (mapping (lambda (x) x)))
  "(Array->vector array (mapping identity)) converts a Godot Array to a Scheme vector, optionally mapping each element via the given mapping function."
  (let* ((size (length array))
         (v (make-vector size)))
    (let loop ((size size))
      (when (> size 0)
        (let ((idx (- size 1)))
          (set! (v idx) (mapping (array idx)))
          (loop idx))))
    v))

(define (Array-for-each-reversed f array)
  "Loops over a Godot array in reverse order."
  (let loop ((size (length array)))
    (when (> size 0)
      (let ((idx (- size 1)))
        (f (array idx))
        (loop idx)))))

(define* (Array->list array (mapping (lambda (x) x)))
  "(Array->list array (mapping identity)) converts a Godot Array to a Scheme list, optionally mapping each element via the given mapping function. #<unspecified> values are removed."
  (let ((res ()))
    (Array-for-each-reversed
     (lambda (a)
       (let ((e (mapping a)))
         (unless (unspecified? e)
           (set! res (cons e res)))))
     array)
    res))

