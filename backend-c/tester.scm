(use matchable fmt fmt-c clojurian-syntax)

(define max-ticks 100000000)

;; match the generated identifiers from the source files.
;; TODO: don't just copy-paste this!
(define (sconc . args) (string->symbol (fold conc "" (reverse args))))
(define (identifier op) (->> `(("_" . "__") ("else" . "_else") ("." . "_") ("+" . "_plus_") ("-" . "_minus_") ("*" . "_star_") ("/" . "_divide_") (">" . "_gt_") ("<" . "_lt_") ("%" . "_percent_")) (string-translate* (conc op)) (string->symbol)))
(define (op->identifier op) (sconc "OP_" (string-upcase (symbol->string (identifier op)))))


;; (define program
;;   (random-code
;;     40
;;    `(
;;      #f #t 0 1 2 3 4 5 6 7 8 9
;;      boolean.and boolean.frominteger boolean.not boolean.or
;;      exec.do*count exec.do*range exec.do*times exec.if integer.%
;;      integer.* integer.+ ;; integer.-
;;      integer./ integer.< integer.> integer.dup
;;      integer.fromboolean integer.max integer.min integer.pop integer.rot)))

(define program '(1 1000000 exec.do*times (1 integer.+)))


(system (conc "mzscheme ../schush.ss '" program "'"))

;; (define program '(1.0 30000 exec.do*times (1.0001 float.*) integer.fromfloat))

;; (define program '((integer.< (exec.if (integer.rot) ((integer.+) (exec.do*range (3 (2))) (integer./ (integer.dup)) 7) (exec.do*range)) (exec.do*count))))

;; rewrite an object construct to C code
(define (rew body)
  (match body
    ((head rest ...)
     `(m_obj_cons ,(rew head) ,(rew rest)))
    (()
     `(& the_empty_list))
    ((? fixnum? n)
     `(m_obj_from_integer ,n))
    ((? flonum? n)
     `(m_obj_from_float ,n))
    ((? symbol? op)
     `(m_obj_from_op ,(op->identifier op)))
    ((and (or #f #t) x)
     `(m_obj_from_boolean ,(if x 1 0)))
    (else (error "cant mach" body))))

;; every written C in Lisp before? It's not great ...
(define main
  `(%begin


    (%include "time.h")
    (%include "stdint.h")
    ,(dsp "
 double timespecDiff(struct timespec *timeA_p, struct timespec *timeB_p)
 { return (((timeA_p->tv_sec * 1000000000) + timeA_p->tv_nsec) -
         ((timeB_p->tv_sec * 1000000000) + timeB_p->tv_nsec)) / 1000000.0; }")

    (%include "pushc.c")
    (%fun int main ()
          (%var m_machine_t _cpu)
          (%var m_machine_t (* cpu) (& _cpu))
          (m_machine_init cpu)

          (m_stack_exec_push cpu ,(rew program))

          (printf "starting:    (") (m_exec_print cpu)    (printf "\n")
          (printf "\n")

          (%var int ticks 0)
          (%var double elap)

          ,(cat "struct timespec start, end;")
           (clock_gettime CLOCK_MONOTONIC (& start))

          (while (&& (m_apply cpu) (< ticks ,max-ticks))
            (++ ticks))

          (clock_gettime CLOCK_MONOTONIC (& end))
          (= elap (timespecDiff (& end) (& start)))

          (printf "integer: (") (m_integer_print cpu)(printf ")\n")
          (printf "boolean: (") (m_boolean_print cpu)(printf ")\n")
          (printf "exec:    (") (m_exec_print cpu)   (printf ")\n")
          (printf "\n")
          (printf "ticks:   %d\n" ticks)
          (printf "elapsed: %.1fms\n" elap)
          (printf "\n")

          (m_machine_free cpu)
          0)))



(with-output-to-file (car (command-line-arguments))
  (lambda () (fmt #t (c-expr main))))
