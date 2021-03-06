/*
autogenerated from ("push-core.scm" "push-float.scm") with backend.c.scm

m_op_t should be defined as some integer (switchable) type

 you'll need to implement these functions:

 m_typeof_obj
 m_stack_integer_push
 m_stack_integer_pop
 m_stack_integer_length
 m_stack_float_push
 m_stack_float_pop
 m_stack_float_length
 m_stack_exec_push
 m_stack_exec_pop
 m_stack_exec_length
 m_stack_boolean_push
 m_stack_boolean_pop
 m_stack_boolean_length
 m_obj_to_integer
 m_obj_to_float
 m_obj_to_boolean
 m_obj_from_op
 m_obj_from_integer
 m_integer_from_float
 m_integer_from_boolean
 m_float_from_integer
 m_boolean_from_integer
 m_boolean_from_float*/

int m_max (int a, int b) {
    if (a > b) {
        return a;
    } else {
        return b;
    }
}
int m_min (int a, int b) {
    if (a < b) {
        return a;
    } else {
        return b;
    }
}

#define OP_BOOLEAN_AND 8
#define OP_BOOLEAN_FROMINTEGER 5
#define OP_BOOLEAN_NOT 6
#define OP_BOOLEAN_OR 7
#define OP_EXEC_DO_STAR_COUNT 2
#define OP_EXEC_DO_STAR_RANGE 3
#define OP_EXEC_DO_STAR_TIMES 1
#define OP_EXEC_IF 4
#define OP_FLOAT__STAR_ 29
#define OP_FLOAT__PLUS_ 31
#define OP_FLOAT__MINUS_ 30
#define OP_FLOAT__DIVIDE_ 28
#define OP_FLOAT__LT_ 27
#define OP_FLOAT__GT_ 26
#define OP_FLOAT_DUP 34
#define OP_FLOAT_FROMINTEGER 23
#define OP_FLOAT_MAX 24
#define OP_FLOAT_MIN 25
#define OP_FLOAT_POP 35
#define OP_FLOAT_ROT 32
#define OP_FLOAT_SWAP 33
#define OP_INTEGER__PERCENT_ 14
#define OP_INTEGER__STAR_ 16
#define OP_INTEGER__PLUS_ 18
#define OP_INTEGER__MINUS_ 17
#define OP_INTEGER__DIVIDE_ 15
#define OP_INTEGER__LT_ 13
#define OP_INTEGER__GT_ 12
#define OP_INTEGER_DUP 20
#define OP_INTEGER_FROMBOOLEAN 9
#define OP_INTEGER_FROMFLOAT 22
#define OP_INTEGER_MAX 10
#define OP_INTEGER_MIN 11
#define OP_INTEGER_POP 21
#define OP_INTEGER_ROT 19

char * lookup (m_op_t op) {
    switch (op) {
        case OP_BOOLEAN_AND:
            return "boolean.and";
        case OP_BOOLEAN_FROMINTEGER:
            return "boolean.frominteger";
        case OP_BOOLEAN_NOT:
            return "boolean.not";
        case OP_BOOLEAN_OR:
            return "boolean.or";
        case OP_EXEC_DO_STAR_COUNT:
            return "exec.do*count";
        case OP_EXEC_DO_STAR_RANGE:
            return "exec.do*range";
        case OP_EXEC_DO_STAR_TIMES:
            return "exec.do*times";
        case OP_EXEC_IF:
            return "exec.if";
        case OP_FLOAT__STAR_:
            return "float.*";
        case OP_FLOAT__PLUS_:
            return "float.+";
        case OP_FLOAT__MINUS_:
            return "float.-";
        case OP_FLOAT__DIVIDE_:
            return "float./";
        case OP_FLOAT__LT_:
            return "float.<";
        case OP_FLOAT__GT_:
            return "float.>";
        case OP_FLOAT_DUP:
            return "float.dup";
        case OP_FLOAT_FROMINTEGER:
            return "float.frominteger";
        case OP_FLOAT_MAX:
            return "float.max";
        case OP_FLOAT_MIN:
            return "float.min";
        case OP_FLOAT_POP:
            return "float.pop";
        case OP_FLOAT_ROT:
            return "float.rot";
        case OP_FLOAT_SWAP:
            return "float.swap";
        case OP_INTEGER__PERCENT_:
            return "integer.%";
        case OP_INTEGER__STAR_:
            return "integer.*";
        case OP_INTEGER__PLUS_:
            return "integer.+";
        case OP_INTEGER__MINUS_:
            return "integer.-";
        case OP_INTEGER__DIVIDE_:
            return "integer./";
        case OP_INTEGER__LT_:
            return "integer.<";
        case OP_INTEGER__GT_:
            return "integer.>";
        case OP_INTEGER_DUP:
            return "integer.dup";
        case OP_INTEGER_FROMBOOLEAN:
            return "integer.fromboolean";
        case OP_INTEGER_FROMFLOAT:
            return "integer.fromfloat";
        case OP_INTEGER_MAX:
            return "integer.max";
        case OP_INTEGER_MIN:
            return "integer.min";
        case OP_INTEGER_POP:
            return "integer.pop";
        case OP_INTEGER_ROT:
            return "integer.rot";
        default:
            return "?";
    }
}


// exec.do*times
// ((((todo) exec) ((num-times) integer) (abort (< num-times 1)))
//  (push exec (cons 'integer.pop todo))
//  (push exec 'exec.do*range)
//  (push exec (- num-times 1))
//  (push exec 0))
void exec_do_star_times (m_machine_t *m) {
    if (m_stack_exec_length(m) < 1 || m_stack_integer_length(m) < 1) {
        return;
    }
    m_exec_t todo = m_stack_exec_pop(m);
    m_integer_t num_minus_times = m_stack_integer_pop(m);
    if (num_minus_times < 1) {
        m_stack_integer_push(m, num_minus_times);
        m_stack_exec_push(m, todo);
        return;
    }
    m_stack_exec_push(m, m_obj_cons(m_obj_from_op(OP_INTEGER_POP), todo));
    m_stack_exec_push(m, m_obj_from_op(OP_EXEC_DO_STAR_RANGE));
    m_stack_exec_push(m, m_obj_from_integer((num_minus_times - 1)));
    m_stack_exec_push(m, m_obj_from_integer(0));
}

// exec.do*count
// ((((todo) exec) ((num-times) integer) (abort (< num-times 1)))
//  (push exec todo)
//  (push exec 'exec.do*range)
//  (push exec (- num-times 1))
//  (push exec 0))
void exec_do_star_count (m_machine_t *m) {
    if (m_stack_exec_length(m) < 1 || m_stack_integer_length(m) < 1) {
        return;
    }
    m_exec_t todo = m_stack_exec_pop(m);
    m_integer_t num_minus_times = m_stack_integer_pop(m);
    if (num_minus_times < 1) {
        m_stack_integer_push(m, num_minus_times);
        m_stack_exec_push(m, todo);
        return;
    }
    m_stack_exec_push(m, todo);
    m_stack_exec_push(m, m_obj_from_op(OP_EXEC_DO_STAR_RANGE));
    m_stack_exec_push(m, m_obj_from_integer((num_minus_times - 1)));
    m_stack_exec_push(m, m_obj_from_integer(0));
}

// exec.do*range
// ((((todo) exec) ((destination current) integer) (abort (< destination 1)))
//  (push integer current)
//  (if (= current destination)
//    (void)
//    (begin
//      (push exec todo)
//      (push exec 'exec.do*range)
//      (push exec destination)
//      (push exec (if (> current destination) (- current 1) (+ current 1)))))
//  (push exec todo))
void exec_do_star_range (m_machine_t *m) {
    if (m_stack_exec_length(m) < 1 || m_stack_integer_length(m) < 2) {
        return;
    }
    m_exec_t todo = m_stack_exec_pop(m);
    m_integer_t destination = m_stack_integer_pop(m);
    m_integer_t current = m_stack_integer_pop(m);
    if (destination < 1) {
        m_stack_integer_push(m, current);
        m_stack_integer_push(m, destination);
        m_stack_exec_push(m, todo);
        return;
    }
    m_stack_integer_push(m, current);
    if (current == destination) {
        
    } else {
        m_stack_exec_push(m, todo);
        m_stack_exec_push(m, m_obj_from_op(OP_EXEC_DO_STAR_RANGE));
        m_stack_exec_push(m, m_obj_from_integer(destination));
        m_stack_exec_push(m,
                          m_obj_from_integer((current > destination) ? (current
                                                                          - 1) : (current
                                                                                    + 1)));
    }
    m_stack_exec_push(m, todo);
}

// exec.if
// ((((then else) exec) ((test) boolean))
//  (if test (push exec then) (push exec else)))
void exec_if (m_machine_t *m) {
    if (m_stack_exec_length(m) < 2 || m_stack_boolean_length(m) < 1) {
        return;
    }
    m_exec_t then = m_stack_exec_pop(m);
    m_exec_t _else = m_stack_exec_pop(m);
    m_boolean_t test = m_stack_boolean_pop(m);
    if (test) {
        m_stack_exec_push(m, then);
    } else {
        m_stack_exec_push(m, _else);
    }
}

// boolean.frominteger
// ((((i0) integer)) (push boolean i0))
void boolean_frominteger (m_machine_t *m) {
    if (m_stack_integer_length(m) < 1) {
        return;
    }
    m_integer_t i0 = m_stack_integer_pop(m);
    m_stack_boolean_push(m, m_boolean_from_integer(i0));
}

// boolean.not
// ((((b0) boolean)) (push boolean (not b0)))
void boolean_not (m_machine_t *m) {
    if (m_stack_boolean_length(m) < 1) {
        return;
    }
    m_boolean_t b0 = m_stack_boolean_pop(m);
    m_stack_boolean_push(m, !(b0));
}

// boolean.or
// ((((b0 b1) boolean)) (push boolean (or b0 b1)))
void boolean_or (m_machine_t *m) {
    if (m_stack_boolean_length(m) < 2) {
        return;
    }
    m_boolean_t b0 = m_stack_boolean_pop(m);
    m_boolean_t b1 = m_stack_boolean_pop(m);
    m_stack_boolean_push(m, (b0 || b1));
}

// boolean.and
// ((((b0 b1) boolean)) (push boolean (and b0 b1)))
void boolean_and (m_machine_t *m) {
    if (m_stack_boolean_length(m) < 2) {
        return;
    }
    m_boolean_t b0 = m_stack_boolean_pop(m);
    m_boolean_t b1 = m_stack_boolean_pop(m);
    m_stack_boolean_push(m, (b0 && b1));
}

// integer.fromboolean
// ((((b) boolean)) (push integer b))
void integer_fromboolean (m_machine_t *m) {
    if (m_stack_boolean_length(m) < 1) {
        return;
    }
    m_boolean_t b = m_stack_boolean_pop(m);
    m_stack_integer_push(m, m_integer_from_boolean(b));
}

// integer.max
// ((((i0 i1) integer)) (push integer (max i0 i1)))
void integer_max (m_machine_t *m) {
    if (m_stack_integer_length(m) < 2) {
        return;
    }
    m_integer_t i0 = m_stack_integer_pop(m);
    m_integer_t i1 = m_stack_integer_pop(m);
    m_stack_integer_push(m, m_max(i0, i1));
}

// integer.min
// ((((i0 i1) integer)) (push integer (min i0 i1)))
void integer_min (m_machine_t *m) {
    if (m_stack_integer_length(m) < 2) {
        return;
    }
    m_integer_t i0 = m_stack_integer_pop(m);
    m_integer_t i1 = m_stack_integer_pop(m);
    m_stack_integer_push(m, m_min(i0, i1));
}

// integer.>
// ((((e0 e1) integer)) (push boolean (> e1 e0)))
void integer__gt_ (m_machine_t *m) {
    if (m_stack_integer_length(m) < 2) {
        return;
    }
    m_integer_t e0 = m_stack_integer_pop(m);
    m_integer_t e1 = m_stack_integer_pop(m);
    m_stack_boolean_push(m, m_boolean_from_integer((e1 > e0)));
}

// integer.<
// ((((e0 e1) integer)) (push boolean (< e1 e0)))
void integer__lt_ (m_machine_t *m) {
    if (m_stack_integer_length(m) < 2) {
        return;
    }
    m_integer_t e0 = m_stack_integer_pop(m);
    m_integer_t e1 = m_stack_integer_pop(m);
    m_stack_boolean_push(m, m_boolean_from_integer((e1 < e0)));
}

// integer.%
// ((((e0 e1) integer) (abort (= e0 0))) (push integer (modulo e1 e0)))
void integer__percent_ (m_machine_t *m) {
    if (m_stack_integer_length(m) < 2) {
        return;
    }
    m_integer_t e0 = m_stack_integer_pop(m);
    m_integer_t e1 = m_stack_integer_pop(m);
    if (e0 == 0) {
        m_stack_integer_push(m, e1);
        m_stack_integer_push(m, e0);
        return;
    }
    m_stack_integer_push(m, (e1 % e0));
}

// integer./
// ((((e0 e1) integer) (abort (= e0 0))) (push integer (/ e1 e0)))
void integer__divide_ (m_machine_t *m) {
    if (m_stack_integer_length(m) < 2) {
        return;
    }
    m_integer_t e0 = m_stack_integer_pop(m);
    m_integer_t e1 = m_stack_integer_pop(m);
    if (e0 == 0) {
        m_stack_integer_push(m, e1);
        m_stack_integer_push(m, e0);
        return;
    }
    m_stack_integer_push(m, (e1 / e0));
}

// integer.*
// ((((e0 e1) integer)) (push integer (* e0 e1)))
void integer__star_ (m_machine_t *m) {
    if (m_stack_integer_length(m) < 2) {
        return;
    }
    m_integer_t e0 = m_stack_integer_pop(m);
    m_integer_t e1 = m_stack_integer_pop(m);
    m_stack_integer_push(m, (e0 * e1));
}

// integer.-
// ((((e0 e1) integer)) (push integer (- e0 e1)))
void integer__minus_ (m_machine_t *m) {
    if (m_stack_integer_length(m) < 2) {
        return;
    }
    m_integer_t e0 = m_stack_integer_pop(m);
    m_integer_t e1 = m_stack_integer_pop(m);
    m_stack_integer_push(m, (e0 - e1));
}

// integer.+
// ((((e0 e1) integer)) (push integer (+ e0 e1)))
void integer__plus_ (m_machine_t *m) {
    if (m_stack_integer_length(m) < 2) {
        return;
    }
    m_integer_t e0 = m_stack_integer_pop(m);
    m_integer_t e1 = m_stack_integer_pop(m);
    m_stack_integer_push(m, (e0 + e1));
}

// integer.rot
// ((((e0 e1 e2) integer)) (push integer e1) (push integer e0) (push integer e2))
void integer_rot (m_machine_t *m) {
    if (m_stack_integer_length(m) < 3) {
        return;
    }
    m_integer_t e0 = m_stack_integer_pop(m);
    m_integer_t e1 = m_stack_integer_pop(m);
    m_integer_t e2 = m_stack_integer_pop(m);
    m_stack_integer_push(m, e1);
    m_stack_integer_push(m, e0);
    m_stack_integer_push(m, e2);
}

// integer.dup
// ((((e0) integer)) (push integer e0) (push integer e0))
void integer_dup (m_machine_t *m) {
    if (m_stack_integer_length(m) < 1) {
        return;
    }
    m_integer_t e0 = m_stack_integer_pop(m);
    m_stack_integer_push(m, e0);
    m_stack_integer_push(m, e0);
}

// integer.pop
// ((((e0) integer)) (void))
void integer_pop (m_machine_t *m) {
    if (m_stack_integer_length(m) < 1) {
        return;
    }
    m_integer_t e0 = m_stack_integer_pop(m);
    
}

// integer.fromfloat
// ((((f0) float)) (push integer f0))
void integer_fromfloat (m_machine_t *m) {
    if (m_stack_float_length(m) < 1) {
        return;
    }
    m_float_t f0 = m_stack_float_pop(m);
    m_stack_integer_push(m, m_integer_from_float(f0));
}

// float.frominteger
// ((((f0) integer)) (push float f0))
void float_frominteger (m_machine_t *m) {
    if (m_stack_integer_length(m) < 1) {
        return;
    }
    m_integer_t f0 = m_stack_integer_pop(m);
    m_stack_float_push(m, m_float_from_integer(f0));
}

// float.max
// ((((i0 i1) float)) (push float (max i0 i1)))
void float_max (m_machine_t *m) {
    if (m_stack_float_length(m) < 2) {
        return;
    }
    m_float_t i0 = m_stack_float_pop(m);
    m_float_t i1 = m_stack_float_pop(m);
    m_stack_float_push(m, m_max(i0, i1));
}

// float.min
// ((((f0 f1) float)) (push float (min f0 f1)))
void float_min (m_machine_t *m) {
    if (m_stack_float_length(m) < 2) {
        return;
    }
    m_float_t f0 = m_stack_float_pop(m);
    m_float_t f1 = m_stack_float_pop(m);
    m_stack_float_push(m, m_min(f0, f1));
}

// float.>
// ((((f0 f1) integer)) (push boolean (> f1 f0)))
void float__gt_ (m_machine_t *m) {
    if (m_stack_integer_length(m) < 2) {
        return;
    }
    m_integer_t f0 = m_stack_integer_pop(m);
    m_integer_t f1 = m_stack_integer_pop(m);
    m_stack_boolean_push(m, m_boolean_from_integer((f1 > f0)));
}

// float.<
// ((((f0 f1) float)) (push boolean (< f1 f0)))
void float__lt_ (m_machine_t *m) {
    if (m_stack_float_length(m) < 2) {
        return;
    }
    m_float_t f0 = m_stack_float_pop(m);
    m_float_t f1 = m_stack_float_pop(m);
    m_stack_boolean_push(m, m_boolean_from_float((f1 < f0)));
}

// float./
// ((((f0 f1) float) (abort (= f0 0))) (push float (/ f1 f0)))
void float__divide_ (m_machine_t *m) {
    if (m_stack_float_length(m) < 2) {
        return;
    }
    m_float_t f0 = m_stack_float_pop(m);
    m_float_t f1 = m_stack_float_pop(m);
    if (f0 == 0) {
        m_stack_float_push(m, f1);
        m_stack_float_push(m, f0);
        return;
    }
    m_stack_float_push(m, (f1 / f0));
}

// float.*
// ((((f0 f1) float)) (push float (* f0 f1)))
void float__star_ (m_machine_t *m) {
    if (m_stack_float_length(m) < 2) {
        return;
    }
    m_float_t f0 = m_stack_float_pop(m);
    m_float_t f1 = m_stack_float_pop(m);
    m_stack_float_push(m, (f0 * f1));
}

// float.-
// ((((f0 f1) float)) (push float (- f0 f1)))
void float__minus_ (m_machine_t *m) {
    if (m_stack_float_length(m) < 2) {
        return;
    }
    m_float_t f0 = m_stack_float_pop(m);
    m_float_t f1 = m_stack_float_pop(m);
    m_stack_float_push(m, (f0 - f1));
}

// float.+
// ((((f0 f1) float)) (push float (+ f0 f1)))
void float__plus_ (m_machine_t *m) {
    if (m_stack_float_length(m) < 2) {
        return;
    }
    m_float_t f0 = m_stack_float_pop(m);
    m_float_t f1 = m_stack_float_pop(m);
    m_stack_float_push(m, (f0 + f1));
}

// float.rot
// ((((f0 f1 f2) float)) (push float f1) (push float f0) (push float f2))
void float_rot (m_machine_t *m) {
    if (m_stack_float_length(m) < 3) {
        return;
    }
    m_float_t f0 = m_stack_float_pop(m);
    m_float_t f1 = m_stack_float_pop(m);
    m_float_t f2 = m_stack_float_pop(m);
    m_stack_float_push(m, f1);
    m_stack_float_push(m, f0);
    m_stack_float_push(m, f2);
}

// float.swap
// ((((f0 f1) float)) (push float f0) (push float f1))
void float_swap (m_machine_t *m) {
    if (m_stack_float_length(m) < 2) {
        return;
    }
    m_float_t f0 = m_stack_float_pop(m);
    m_float_t f1 = m_stack_float_pop(m);
    m_stack_float_push(m, f0);
    m_stack_float_push(m, f1);
}

// float.dup
// ((((f0) float)) (push float f0) (push float f0))
void float_dup (m_machine_t *m) {
    if (m_stack_float_length(m) < 1) {
        return;
    }
    m_float_t f0 = m_stack_float_pop(m);
    m_stack_float_push(m, f0);
    m_stack_float_push(m, f0);
}

// float.pop
// ((((f0) float)) (void))
void float_pop (m_machine_t *m) {
    if (m_stack_float_length(m) < 1) {
        return;
    }
    m_float_t f0 = m_stack_float_pop(m);
    
}

int m_apply_op (m_machine_t *m, m_op_t op) {
    switch (op) {
        case OP_BOOLEAN_AND:
            boolean_and(m);
            break;
        case OP_BOOLEAN_FROMINTEGER:
            boolean_frominteger(m);
            break;
        case OP_BOOLEAN_NOT:
            boolean_not(m);
            break;
        case OP_BOOLEAN_OR:
            boolean_or(m);
            break;
        case OP_EXEC_DO_STAR_COUNT:
            exec_do_star_count(m);
            break;
        case OP_EXEC_DO_STAR_RANGE:
            exec_do_star_range(m);
            break;
        case OP_EXEC_DO_STAR_TIMES:
            exec_do_star_times(m);
            break;
        case OP_EXEC_IF:
            exec_if(m);
            break;
        case OP_FLOAT__STAR_:
            float__star_(m);
            break;
        case OP_FLOAT__PLUS_:
            float__plus_(m);
            break;
        case OP_FLOAT__MINUS_:
            float__minus_(m);
            break;
        case OP_FLOAT__DIVIDE_:
            float__divide_(m);
            break;
        case OP_FLOAT__LT_:
            float__lt_(m);
            break;
        case OP_FLOAT__GT_:
            float__gt_(m);
            break;
        case OP_FLOAT_DUP:
            float_dup(m);
            break;
        case OP_FLOAT_FROMINTEGER:
            float_frominteger(m);
            break;
        case OP_FLOAT_MAX:
            float_max(m);
            break;
        case OP_FLOAT_MIN:
            float_min(m);
            break;
        case OP_FLOAT_POP:
            float_pop(m);
            break;
        case OP_FLOAT_ROT:
            float_rot(m);
            break;
        case OP_FLOAT_SWAP:
            float_swap(m);
            break;
        case OP_INTEGER__PERCENT_:
            integer__percent_(m);
            break;
        case OP_INTEGER__STAR_:
            integer__star_(m);
            break;
        case OP_INTEGER__PLUS_:
            integer__plus_(m);
            break;
        case OP_INTEGER__MINUS_:
            integer__minus_(m);
            break;
        case OP_INTEGER__DIVIDE_:
            integer__divide_(m);
            break;
        case OP_INTEGER__LT_:
            integer__lt_(m);
            break;
        case OP_INTEGER__GT_:
            integer__gt_(m);
            break;
        case OP_INTEGER_DUP:
            integer_dup(m);
            break;
        case OP_INTEGER_FROMBOOLEAN:
            integer_fromboolean(m);
            break;
        case OP_INTEGER_FROMFLOAT:
            integer_fromfloat(m);
            break;
        case OP_INTEGER_MAX:
            integer_max(m);
            break;
        case OP_INTEGER_MIN:
            integer_min(m);
            break;
        case OP_INTEGER_POP:
            integer_pop(m);
            break;
        case OP_INTEGER_ROT:
            integer_rot(m);
            break;
        default:
            return 0;
            break;
    }
    return 1;
}

int m_apply_literal (m_machine_t *m, m_exec_t literal) {
    switch (m_typeof_obj(literal)) {
        case M_TYPE_INTEGER:
            m_stack_integer_push(m, m_obj_to_integer(literal));
            break;
        case M_TYPE_BOOLEAN:
            m_stack_boolean_push(m, m_obj_to_boolean(literal));
            break;
        case M_TYPE_FLOAT:
            m_stack_float_push(m, m_obj_to_float(literal));
            break;
    }
    return 1;
}

