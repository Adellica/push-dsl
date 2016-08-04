;;; define push's core instructions using a simple stack-oriented DSL.
;;;
;;; an instruction pops off values based on the first form:
;;; (((variables ...) stack) stacks ...)
;;;
;;;
;;; the only allowed procedures are typically just the common
;;; operators and primitive functions supported by all operating
;;; systems, like + - % /


;; ==================== integer

(define-instruction integer.pop
  ( ((e0) integer) )
  (void))

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
  ( ((e0 e1) integer) )
  (push integer (/ e1 e0)))

(define-instruction integer.%
  ( ((e0 e1) integer) )
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
;;   ( ((yank-index) exec) )
;;   (push exec (peek exec yank-index)))

(define-instruction exec.if
  ( ((then otherwise) exec)
    ((test)           boolean) )
  (if test
      (push exec then)
      (push exec otherwise)))

;; TODO
;; (define-instruction exec.length
;;   ( ((e0) exec) )
;;   (push exec (if (pair? e0)
;;                  (length e0)
;;                  1)))

;; TODO: why do we need `copy-tree`?
(define-instruction exec.do*range
  ( ((todo) exec)
    ((destination current) integer) )
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
    ((num-times) integer) )
  (push exec todo)
  (push exec 'exec.do*range)
  (push exec (- num-times 1))
  (push exec 0))

(define-instruction exec.do*times
  ( ((todo) exec)
    ((num-times) integer) )
  (push exec (cons 'integer.pop todo))
  (push exec 'exec.do*range)
  (push exec (- num-times 1))
  (push exec 0))


