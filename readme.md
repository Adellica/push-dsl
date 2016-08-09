  [Push3]: http://faculty.hampshire.edu/lspector/push.html
  [DSL]: https://en.wikipedia.org/wiki/Domain-specific_language

## Push3 for C

This project tries to do two things:

- Define Lee Specter's [Push3] in its own stack-oriented [DSL] language
- Implement the neccessary machinery to have a C implementation of
  [Push3] working

Project state: Experimental / Alpha

## Push DSL

One goal is to define the Push instructions themselves in a language
suited to describe a lot of pushing and popping. For example, consider
these Push instruction definitions:

```scheme
(define-instruction integer.pop
  ( ((e0) integer) )
  (void))

(define-instruction integer.dup
  ( ((e0) integer) )
  (push integer e0)
  (push integer e0))

(define-instruction integer.+
  ( ((e0 e1) integer) )
  (push integer (+ e0 e1)))
```

Hopefully, those are self-explanatory. These definitions can in turn
be used to generate an interpreter for Push in various languages. They
take the original `define-registered` (see `schush.ss`) macro one step
further.

The define-instruction semantics is as follows:

    (define-instruction name specifications body ...)

Where:

- `name` is the name of the instruction that's being defined (with
  type)
- `specifications` is a list of `(elements stack)`
- `stack` is one of the machine's stacks (integer, boolean)
- `elements` is a list of variable names binding that name to the
       stack's popped values. The first variable in `elements` is
       bound to the first popped, and so on.

An instruction pops off values implicitly based on the
`specifications` form:

    (((variables ...) stack) stacks ...)

If there are more variables in `elements` than are available on their
stack, `body` is not executed conforming to Push's `noop` behaviour.

The body definition consists of a limited set of allowed
procedures. It is written in a mutable style (after all, the
stack-based machine does operate by mutating its stack). Note that all
the procedures used in the body must be "easily" implementable in all
backend languages.

That means that the body definition can only contain the stack
operators (eg. push peek yank) and some primitives supported by
everyone (eg. + - * / %). Exactly which ones should probably be
formalised at some stage.

The machine's stacks are directly available in a body as their
variable names (integer, exec, boolean, ...). So (push integer 1) will
push 1 onto the integer stack. It's possible for a body to be defined
as (push char 3), in which case the backends must handle this type
conversion.

The `specifications` in this style produce a lot of parenthesis, even
for a lisp! Hopefully, it's still clear and readable.

The DSL push definitions are incomplete, but completeing this stage
should be a relatively simple task.

## C Backend

Have a look in the `backend-c` folder for how the Push DSL is turned
into a `.c` file. A basic runtime is provided too, but is incomplete
(it lacks a GC and doesn't support all of Push's instructions).

I decided to include the
[generated C file](blob/master/backend-c/push-core.scm.gen.c) into
this repository because it is relatively readable (leaning onto GCC
for true optimization) and so you don't have you do get the
[CHICKEN Scheme](http://call-cc.org) tools for "compiling" yourself.

One of the `C`-versions's "pillars" is that all stacks are statically
typed (only integers go on the integer stack), except the code and
exec stack.

### Performance

Unsurprisingly, there is a performance boost when running the
generated C-version of Push against original `schush.ss` in `mzscheme`
directly for the same program inputs. However, the improvement isn't
completely awesome like I'd hoped.  The C-backend is compiled with
`gcc`'s heaviest optimization, and compared with `mzscheme
schush.ss`. This benchmark-constructed program runs, on my machine,
20x faster in `C` than with `mzscheme`:

    (1 1000000 exec.do*times (1 integer.+))

The test is probably very synthetic and I don't know what the
real-world performance boosts would be. Also, 20x faster isn't that
impressive. Note that it seems that
[Clojush](https://github.com/lspector/Clojush), the latest Push3
intepreter from the same authors, is considerably slower:

```clojure
clojush.core=> (require 'clojush.interpreter 'clojush.globals)
nil
clojush.core=> (swap! global-evalpush-limit (fn [x] 100000000)) ;; run for long
clojush.core=> (time (clojush.interpreter/run-push
   '(1 1000000 exec_do*times (1 integer_add)) (clojush.pushstate/make-push-state)))
"Elapsed time: 26800 msecs"
```

The `mzscheme` version above is approximately 10x faster than this,
giving the `C` version 200x boost from the Clojure implementation.
Still, the usefulness of having a C-based interpreter for Push is
questionable. The Push-programs themselves should probably still be
manipulated (mutated etc) from a Lispish language, but obviously a lot
of flexibility and robustness in the interpreter is lost when we move
to C. And adding new instructions means implementing them in `C` too.

## Scheme backend

I never got around to completing this, but the idea is very simple:
the `define-instruction` described above can be easily implemented as
a macro. See `backend-scheme` for how to start with this.

In terms on benchmarking, it would be interesting to see if
implementing the stacks as
[srfi 4](http://srfi.schemers.org/srfi-4/srfi-4.html) vectors, with
their homogeneous types, might yield some performance boosts without
loosing any flexibility of the language like you do in the C-based
version.

## Conclusion

Generating "fast" C-code from a Push3 DSL didn't yield the performance
boost that I'd hoped for. However, implementing Push3 on top of this
DSL provides, in my opinion, a clearer and more elegant code. Perhaps
learning Push3 itself might be easier if its instructions were all
defined this way.

```scheme
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
```


This has been a fun excercise and good learning experience! Please let
me know if this was of use to you.
