all: push-core.scm.gen.c push-float.scm.gen.c

push-core.scm.gen.c: backend.c.scm push-core.scm push-float.scm
	csi -s backend.c.scm push-core.scm push-float.scm

test.c: tester.scm
	csi -s tester.scm test.c

test:	test.c
	gcc -O9 ./test.c -o test

new-test:
	rm test.c
