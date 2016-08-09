;;;
;;; define push's core instructions using a simple stack-oriented DSL.
;;;


;; ==================== integer

(define-instruction integer.pop
  ( ((e0) integer) )
  (void))

;; here it might be tempting to do
;; (define-instruction integer.dup
;;   ()
;;   (push integer (peek integer 0)))
;; but that's a no-go since the the integer
;; stack might be empty.
(define-instruction integer.dup
  ( ((e0) integer) )
  (push integer e0)
  (push integer e0))

(define-instruction integer.rot
  ( ((e0 e1 e2) integer) )
  (push integer e1)
  (push integer e0)
  (push integer e2))

(define-instruction integer.+
  ( ((e0 e1) integer) )
  (push integer (+ e0 e1)))

(define-instruction integer.-
  ( ((e0 e1) integer) )
  (push integer (- e0 e1)))

(define-instruction integer.*
  ( ((e0 e1) integer) )
  (push integer (* e0 e1)))

(define-instruction integer./
  ( ((e0 e1) integer)
    (abort (= e0 0)))
  (push integer (/ e1 e0)))

(define-instruction integer.%
  ( ((e0 e1) integer)
    (abort (= e0 0)) )
  (push integer (modulo e1 e0)))

(define-instruction integer.<
  ( ((e0 e1) integer) )
  (push boolean (< e1 e0)))

(define-instruction integer.>
  ( ((e0 e1) integer) )
  (push boolean (> e1 e0)))

(define-instruction integer.min
  ( ((i0 i1) integer) )
  (push integer (min i0 i1)))

(define-instruction integer.max
  ( ((i0 i1) integer) )
  (push integer (max i0 i1)))

(define-instruction integer.fromboolean
  ( ((b) boolean) )
  (push integer b))

;; (define-instruction integer.yankdup
;;   ( ((yank-index) integer) )
;;   (push integer (peek integer yank-index)))

;; ==================== boolean

(define-instruction boolean.and
  ( ((b0 b1) boolean) )
  (push boolean (and b0 b1)))

(define-instruction boolean.or
  ( ((b0 b1) boolean) )
  (push boolean (or b0 b1)))

(define-instruction boolean.not
  ( ((b0) boolean) )
  (push boolean (not b0)))

(define-instruction boolean.frominteger
  ( ((i0) integer) )
  (push boolean i0))

;; ==================== exec

;; (define-instruction exec.yankdup
;;   ( ((index) integer) )
;;   (push exec (peek exec index)))

(define-instruction exec.if
  ( ((then else) exec)
    ((test)      boolean) )
  (if test
      (push exec then)
      (push exec else)))

;; TODO
;; (define-instruction exec.length
;;   ( ((e0) exec) )
;;   (push exec (if (pair? e0)
;;                  (length e0)
;;                  1)))

;; TODO: why do we need `copy-tree`?
(define-instruction exec.do*range
  ( ((todo) exec)
    ((destination current) integer)
    (abort (< destination 1) ))
  (push integer current)
  (if (= current destination)
      (void)
      (begin
        (push exec todo)
        (push exec 'exec.do*range)
        (push exec destination)
        (push exec (if (> current destination)
                       (- current 1)
                       (+ current 1)))))
  (push exec todo))

(define-instruction exec.do*count
  ( ((todo) exec)
    ((num-times) integer)
    (abort  (< num-times 1)) )
  (push exec todo)
  (push exec 'exec.do*range)
  (push exec (- num-times 1))
  (push exec 0))

(define-instruction exec.do*times
  ( ((todo) exec)
    ((num-times) integer)
    (abort (< num-times 1)) )
  (push exec (cons 'integer.pop todo))
  (push exec 'exec.do*range)
  (push exec (- num-times 1))
  (push exec 0))


