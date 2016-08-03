all: pushc

push-core.scm.gen.c: backend.c.scm push-core.scm
	csi -s backend.c.scm push-core.scm

pushc: pushc.c push-core.scm.gen.c
	gcc pushc.c -o pushc
