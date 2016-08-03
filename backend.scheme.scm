;; ==================== machine definitions ====================

(define-record machine stack-exec stack-integer stack-boolean)

;; bind variables exec, integer, boolean etc in body to the
;; corresponding machine's stacks. this is deliberately non-hygienic
;; (that's why it has an asterix). because of that, you cannot access
;; nested machine stacks.
(define-syntax with-machine-stacks*
  (er-macro-transformer
   (lambda (x r t)
     (let ((machine (cadr x))
           (body (caddr x)))
       `(let ((machine ,machine))
          (let ((integer (machine-stack-integer machine))
                (exec    (machine-stack-exec machine))
                (boolean (machine-stack-boolean machine))

                (push    (lambda (stack value) (stack-push! stack value)))
                (pop     (lambda (stack)       (stack-pop! stack)))
                (peek    (lambda (stack index) (stack-peek stack (max 0 (min (stack-count stack) index))))))
            ,body)
          machine)))))

(define (create-machine #!key
                        (exec (make-stack))
                        (integer (make-stack))
                        (boolean (make-stack)))
  (define (->stack x) (if (list? x) (list->stack x) x))
  (make-machine (->stack exec) (->stack integer) (->stack boolean)))

;; ;; should print #,(stack (1 2 3)) and #,(stack ())
;; (with-machine-stacks* (create-machine integer: '(1 2 3)) (print integer " and " boolean))


;;; ==================== pop syntax ====================

(define-syntax let-stack%check
  (syntax-rules ()
    ((_ ((vars ...) stack) specs ...)
     (and (>= (stack-count stack) (length '(vars ...)))
          (let-stack%check specs ...)))
    ((_) #t)))

(define-syntax let-pop%exec
  (syntax-rules ()
    ((_ (((var vars ...) stack) specs ...) body ...)
     (let ((var (stack-pop! stack)))
       (let-pop%exec (((vars ...) stack) specs ...) body ...)))
    ((_ ((() stack) specs ...) body ...)
     (let-pop%exec (specs ...) body ...))
    ((_ () body ...)
     (begin body ...))))

;; (let-pop ( ((first second) stack) )
;;     (print "popped: " first second))
(define-syntax let-pop
  (syntax-rules ()
    ((_ ((spec ...) specs ...) body ...)
     (if (let-stack%check (spec ...) specs ...)
         (let-pop%exec ((spec ...) specs ...) body ...)))))

(define-syntax define-instruction
  (syntax-rules ()
    ((_ op definition ...)
     (define (op m)
       (with-machine-stacks* m (let-pop definition ...))))))

(include "push-core.scm")
