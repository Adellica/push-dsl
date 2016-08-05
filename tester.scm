(use schush matchable fmt fmt-c clojurian-syntax)

(define max-ticks 10000)
;; shame on you for copy-pasting, Krish!
(define (sconc . args) (string->symbol (fold conc "" (reverse args))))(define (identifier op) (->> `(("_" . "__") ("else" . "_else") ("." . "_") ("+" . "_plus_") ("-" . "_minus_") ("*" . "_star_") ("/" . "_divide_") (">" . "_gt_") ("<" . "_lt_") ("%" . "_percent_")) (string-translate* (conc op)) (string->symbol))) (define (op->identifier op) (sconc "OP_" (string-upcase (symbol->string (identifier op)))))



(define program
  (random-code
    1000
   `(
     #f #t 0 1 2 3 4 5 6 7 8 9
     boolean.and boolean.frominteger boolean.not boolean.or
     exec.do*count exec.do*range exec.do*times exec.if integer.%
     integer.* integer.+ ;; integer.-
     integer./ integer.< integer.> integer.dup
     integer.fromboolean integer.max integer.min integer.pop integer.rot)))

;;(define program '(1000000 exec.do*times (1 2 integer.> exec.if 100 200 integer.pop)))

(define (print-state state)
  (print "integer: " (get-stack  'integer state))
  (print "float:   " (get-stack  'float state))
  (print "boolean: " (get-stack  'boolean state))
  (print "exec:    " (get-stack  'exec state))
  (print "code:    " (get-stack  'code state))
  (print "ticks:   " (hash-ref   state '_ticks (lambda () #f))))

(define state (make-schush-state))

(print "doing program: " program)
(define start (current-milliseconds))
(run-schush program state tick-limit: max-ticks)
(define elap (- (current-milliseconds) start))

(define (rew body)
  (match body
    ((head rest ...)
     `(m_obj_cons ,(rew head) ,(rew rest)))
    (()
     `(& the_empty_list))
    ((? number? n)
     `(m_obj_from_integer ,n))
    ((? symbol? op)
     `(m_obj_from_op ,(op->identifier op)))
    ((and (or #f #t) x)
     `(m_obj_from_boolean ,(if x 1 0)))
    (else (error "cant mach" body))))

(define (cconc . args)
  (conc "\033[31m" (apply conc args) "\033[0m"))

(define steps 20)

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

          ;;(printf "boolean: (") (m_boolean_print cpu)(printf ")\n")
          ;;(printf "exec:    (") (m_exec_print cpu)   (printf ")\n")
          
          ;; (printf "integer: (") (m_integer_print cpu) (printf "\n")
          ;; (printf "boolean: (") (m_boolean_print cpu)(printf "\n")
          ;; (printf "exec:    (") (m_exec_print cpu)(printf "\n")
          ;; (printf "\n")
          
          (clock_gettime CLOCK_MONOTONIC (& end))
          (= elap (timespecDiff (& end) (& start)))

          (printf "integer: (") (m_integer_print cpu)(printf ")\n")
          (printf "boolean: (") (m_boolean_print cpu)(printf ")\n")
          (printf "  exec:    (") (m_exec_print cpu)   (printf ")\n")
          (printf "ticks:   %d\n" ticks)
          (printf "elapsed: %.1fms\n" elap)
          (printf "\n")

          (printf "=== expected results:\n")
          (printf ,(cconc "integer: " (get-stack 'integer state) "\n"))
          (printf ,(cconc "boolean: " (get-stack 'boolean state) "\n"))
          (printf ,(cconc "exec:    " (get-stack 'exec state) "\n"))
          (printf ,(cconc "ticks:   " (hash-ref state '_ticks) "\n"))
          (printf ,(cconc "elapsed: " elap "ms\n"))
          (printf "speedup is roughly %.0fx\n" (/ ,elap elap))

          (m_machine_free cpu)
          0)))



(with-output-to-file (car (command-line-arguments))
  (lambda () (fmt #t (c-expr main))))
