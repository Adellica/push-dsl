all: pushc

push-core.scm.gen.c: backend.c.scm push-core.scm
	csi -s backend.c.scm push-core.scm

pushc: pushc.c push-core.scm.gen.c
	gcc pushc.c -o pushc


test.c: tester.scm
	csi -s tester.scm test.c

test:	test.c
	gcc -O9 ./test.c -o test

new-test:
	rm test.c
