;;; Push instructions for the "char" type.
;;;
;;; Note: Still quite some work to do on the C-backend for this to
;;; work.

(define-instruction char.pop
  ( ((item) char) )
  (void))

(define-instruction char.dup
  ( ((item) char) )
  (push char item)
  (push char item))

(define-instruction char.swap
  ( ((item0 item1) char) )
  (push char item0)
  (push char item1))

(define-instruction char.rot
  ( ((item0 item1 item2) char) )
  (push char item1)
  (push char item0)
  (push char item2))

(define-instruction char.flush
  ()
  (flush char))

(define-instruction char.eq
  ( ((item0 item1) char) )
  (push boolean (= item0 item1)))


(define-instruction char.stackdepth
  ()
  (push integer (stackdepth char)))

(define-instruction char.yank
  ( ((index) integer) )
  (yank char index))

(define-instruction char.yankdup
  ( ((index) integer) )
  (yankdup char index))

(define-instruction char.shove
  ( ((index) integer)
    ((item)  char) )
  (shove char index item))

(define-instruction char.empty?
  ( ((item) char) )
  (push boolean (empty? char)))


;; there is a lot of hidden semantics in the following
;; declarations. for example, it is the job of each backend to manage
;; the "string" type being pushed onto the "char" stack. note that in
;; Clojush, this instruction is called char_allfromstring.
(define-instruction char.fromstring
  ( ((str) string) )
  (push char str))

;; here too, each backend is responsible for converting an "integer"
;; type to a "char" type. you may want to do a (modulus item 128)
;; here, for example.
(define-instruction char.frominteger
  ( ((item) integer) )
  (push char item))

(define-instruction char.fromfloat
  ( ((item) float) )
  (push char item))

;; each backends must also implement letter?, digit? and whitespace?
;; for the "char" type.
(define-instruction char.isletter
  ( ((item) char) )
  (push boolean (letter? item)))

(define-instruction char.isdigit
  ( ((item) char) )
  (push boolean (digit? item)))

(define-instruction char.iswhitespace
  ( ((item) char) )
  (push boolean (whitespace? item)))

