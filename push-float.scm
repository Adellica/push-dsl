
(define-instruction float.pop
  ( ((f0) float) )
  (void))

(define-instruction float.dup
  ( ((f0) float) )
  (push float f0)
  (push float f0))

(define-instruction float.swap
  ( ((f0 f1) float) )
  (push float f0)
  (push float f1))

(define-instruction float.rot
  ( ((f0 f1 f2) float) )
  (push float f1)
  (push float f0)
  (push float f2))

(define-instruction float.+
  ( ((f0 f1) float) )
  (push float (+ f0 f1)))

(define-instruction float.-
  ( ((f0 f1) float) )
  (push float (- f0 f1)))

(define-instruction float.*
  ( ((f0 f1) float) )
  (push float (* f0 f1)))

(define-instruction float./
  ( ((f0 f1) float)
    (abort (= f0 0)))
  (push float (/ f1 f0)))

(define-instruction float.<
  ( ((f0 f1) float) )
  (push boolean (< f1 f0)))

(define-instruction float.>
  ( ((f0 f1) integer) )
  (push boolean (> f1 f0)))

(define-instruction float.min
  ( ((f0 f1) float) )
  (push float (min f0 f1)))

(define-instruction float.max
  ( ((i0 i1) float) )
  (push float (max i0 i1)))


(define-instruction float.frominteger
  ( ((f0) integer) )
  (push float f0))

(define-instruction integer.fromfloat
  ( ((f0) float) )
  (push integer f0))


;; float.yank
;; float.yankdup
