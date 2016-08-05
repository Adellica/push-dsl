(use matchable fmt fmt-c clojurian-syntax)

(define prefix "tf_")

(define (c-or* . args) ;; like c-or, but handle arity 1
  (if (pair? (cdr args)) (apply c-or args) (car args)))

(define ht-instruction-table (make-hash-table))

;; we need to sort this so that every run produces same results (and
;; it's easier to find your op among the lot)
(define (instruction-table) ;; <-- alist
  (sort (hash-table->alist ht-instruction-table)
        (lambda (a b) (string-ci< (conc a) (conc b)))))

;; list of identifiers that are used. these identifiers need to be
;; provided as c-functions before including generated file. TODO:
;; generate declarations for these functions to help the implementer
(define uses (make-hash-table))
(define (requires identifier)
  (hash-table-set! uses identifier #t)
  identifier)

;; convert from scheme-identifiers to c-identifiers
(define (identifier op)
  (->> `(("_" . "__") ;; <-- no name conflicts
         ("else" . "_else")
         ("." . "_")
         ("+" . "_plus_")
         ("-" . "_minus_")
         ("*" . "_star_")
         ("/" . "_divide_")
         (">" . "_gt_")
         ("<" . "_lt_")
         ("%" . "_percent_"))
       (string-translate* (conc op))
       (string->symbol)))

;; make a legal C identifier. these are #defined in top of file
(define (op->identifier op)
  (sconc "OP_" (string-upcase (symbol->string (identifier op)))))

;; like conc but produce symbol
(define (sconc . args)
  (string->symbol (fold conc "" (reverse args))))

;; keep track of all the types we've encountered. the need for our
;; m_convert_... arise from uses of these types.
(define ht-seen-types (make-hash-table))
(define (seen-types) (hash-table-keys ht-seen-types))
(define (stack-c-type stack)
  (hash-table-set! ht-seen-types stack #t)
  (sconc "m_" stack "_t"))

;; here's something very special of this stack-based machine: all
;; stacks are statically typed (type = name), except the code and the
;; exec stacks which can contain data for all the other stacks.
(define (stack-type stack)
  (case stack
    ((exec code) 'obj)
    (else stack)))

;; call a pop operation on type/stack
(define (pop type)
  `(,(string->symbol (requires (conc "m_stack_" type "_pop"))) m))

;; a stacklaration is a spec which defines variables to be popped and
;; its stack. currently, a spec can be either a stacklaration or an
;; abort statement.
(define stacklaration?
  (match-lambda
   (((var ...) stack) #t)
   (else #f)))

(define (stacklarations specs) (filter stacklaration? specs))

;; (((then else) exec) ((test) boolean))
(define (pop-declare specs)
  `(%begin
    ,@(append-map
       (lambda (spec)
         (let ((stack (cadr spec))) ;; integer, boolean etc
           (map (lambda (var)
                  `(%var ,(stack-c-type stack) ,(identifier var) ,(pop stack)))
                (car spec))))
       (stacklarations specs))))

;; push specs back on stack in reverse order. this should undo the
;; pop-declare safely.
(define (unpop specs)
  `(%begin
    ,@(append-map
       (match-lambda
        (((vars ...) stack)
         (map (lambda (var)
                `(,(requires (sconc "m_stack_" stack "_push")) m ,(identifier var)))
              (reverse vars))))
       (reverse (stacklarations specs)))))

;; produce an expression that passes if there are _NOT_ enough
;; arguments on the set of stacks in specs.
(define (pop-check specs)
  (apply c-or*
         (map
          (lambda (spec)
            (let ((vars (car spec)) (stack (cadr spec)))
              `(< (,(requires (sconc "m_stack_"  stack "_length")) m) ,(length vars))))
          (stacklarations specs))))

;; recursive core of the code generation. this will rewrite bits to
;; fit into C and expand define-instructions and friends.
(define (rewrite body env tt)

  ;; luckily, type inference isn't that hard easy because type == stack
  (define (infer-type e #!optional (env env))
    (or (let infer-type ((e e))
          (cond ((symbol? e) (alist-ref e env))
                ((and (pair? e) (eq? 'quote (car e))) 'op) ;; eg (quote integer.+)
                ((and (pair? e) (eq? 'cons  (car e))) 'obj)
                ((pair? e)   (car (filter-map infer-type (cdr e)))) ;; TODO: length > 1 => error
                ((eq? e #f)  'boolean)
                ((eq? e #t)  'boolean)
                ((fixnum? e) 'integer)
                ((flonum? e) 'float)
                (else #f)))
        (error "unable to infer type " e " in " env)))

  ;; (infer-type '(< current destination) '((current . INT)))

  (match body

    (('define-instruction op spec ...)
     (hash-table-set! ht-instruction-table op (+ 1 (hash-table-size ht-instruction-table)))
     `(%begin
       ;; function comment
       ,(cat nl (columnar "// " (cat op nl (with-output-to-string (lambda () (pp spec))))))
       ;; function body
       (%fun void ,(identifier op) (((%pointer m_machine_t) m))
             ,(rewrite `(let-pop ,@spec) env #f))))

    (('let-pop (specs ...) body ...)

     (let ((env (append
                 (append-map (match-lambda
                              (((vars ...) stack)
                               (map (lambda (var) (cons var stack)) vars)))
                             (stacklarations specs))
                 env)))
       `(%begin (if ,(pop-check specs) (return))
                ,(pop-declare specs)
                ,@(map (match-lambda
                        (('abort condition)
                         `(if ,(rewrite condition env tt)
                              (%begin ,(unpop specs)
                                      (return)))))
                       (filter (lambda (spec) (not (stacklaration? spec))) specs))
                ,(rewrite `(begin ,@body) env #f))))

    (('cons a b) `(m_obj_cons ,(rewrite a env 'obj)
                              ,(rewrite b env 'obj)))

    (('= args ...) `(== ,@(rewrite args env tt)))

    (('or  args ...) (apply c-or* (rewrite args env 'boolean)))
    (('and args ...) `(&& ,@(rewrite args env 'boolean)))
    (('not args ...) `(!  ,@(rewrite args env 'boolean)))

    (('begin body ...) `(%begin ,@(map (cut rewrite <> env #f) body)))
    (('void body ...)  (dsp ""))

    (('quote op)
     (case tt
       ((obj) `(,(requires 'm_obj_from_op) ,(op->identifier op)))
       (else (op->identifier op))))

    ;; ;; quoted symbols are special: they mean the op literal
    ;; (('push stack ('quote op))
    ;;  `(,(requires (sconc "m_stack_" stack "_push")) m
    ;;    (,(requires (sconc "m_convert_"  stack "_from_op"))
    ;;     ,(op->identifier op))))

    (('push stack e)
     (let ((type (stack-type (infer-type e)))
           (stype (stack-type stack)))
       `(,(requires (sconc "m_stack_" stack "_push")) m
         ,(if (eq? type stype)
              ;; type = stack, no conversion needed, but force type
              (rewrite e env type)
              ;; type ≠ stack, only possible for dynamically typed stacks (eg. exec)
              `(,(requires (sconc "m_" stype "_from_" type))
                ,(rewrite e env type))))))

    ((lst ...)  `(,(match (car lst)
                     ('max 'm_max)
                     ('min 'm_min)
                     ('modulo '%)
                     (e e))
                  ,@(map (cut rewrite <> env tt) (cdr lst))))

    ((? symbol? s) (identifier s))

    (else body)))


(unless (pair? (command-line-arguments))
  (print "usage: <push-dsl.scm>\n  eg. csi -s backend.c.scm push-core.scm")
  (exit -1))

(define filename (car (command-line-arguments)))

;; now serialize to C!
(let ((expr (rewrite `(%begin ,@(with-input-from-file filename
                                  (lambda () (port-fold cons '() read)))) '() #f)))

  (with-output-to-file (conc filename ".gen.c")
    (lambda ()

      (define m-apply-op
        `(%fun int m_apply_op (((%pointer m_machine_t) m)
                               (m_op_t op))

               ;;(printf "=== applying op %s\n" (lookup op))
               (switch op
                       ,@(map (lambda (op) `(case ,(op->identifier op) (,(identifier op) m)))
                              (map car (instruction-table)))
                       (default (return 0)))
               1))

      (define m-apply-literal
        `(%fun int m_apply_literal (((%pointer m_machine_t) m)
                                    (m_exec_t literal))
               ;; (printf "=== applying literal %010p " literal) (write literal) (printf "\n")
               (switch ,(c-expr `(,(requires 'm_typeof_obj) literal))
                       ,@(map
                          (lambda (type)
                            `(case ,(sconc "M_TYPE_" (string-upcase (conc type)))
                               (,(requires (sconc "m_stack_" type "_push"))
                                m
                                (,(requires (sconc "m_obj_to_" type))
                                 literal))))
                          (delete 'exec (seen-types))))
               1))

      (fmt #t
           "/// autogenerated from " filename " with backend.c.scm" nl
           "/// " nl
           "/// m_op_t should be defined as some integer (switchable) type" nl
           (columnar "//"
                     (cat " you'll need to implement these functions: "
                          (fmt-join/prefix dsp (-> (hash-table-keys uses)
                                                 (sort (lambda (a b) (string-ci> (conc a) (conc b)))))
                                           "\n ")))
           (c-expr `(%begin (%fun int m_max ((int a) (int b)) (if (> a b) a b))
                            (%fun int m_min ((int a) (int b)) (if (< a b) a b))))
           nl
           (c-expr `(%begin ,@(map (lambda (pair)
                                     `(%define ,(op->identifier (car pair)) ,(cdr pair)))
                                   (instruction-table))))
           nl
           (c-expr `(%fun (%pointer char) lookup ((m_op_t op))
                          (switch op
                                  ,@(map (lambda (op) `(case ,(op->identifier op) ,(symbol->string op)))
                                         (map car (instruction-table)))))) nl
           (c-expr expr) nl


           ;; an op is an atom that names an instruction.
           (c-expr m-apply-op) nl
           (c-expr m-apply-literal) nl
           ))))

