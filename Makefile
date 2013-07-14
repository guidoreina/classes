CC=g++
CXXFLAGS=-g -Wall -pedantic -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -Wno-format -Wno-long-long -I.

LDFLAGS=
LIBS=-lpthread

MAKEDEPEND=${CC} -MM

SKIPLIST_TEST=skiplist_test
INSERT_ONLY_SKIPLIST_TEST=insert_only_skiplist_test
ATOMIC_MARKABLE_PTR_TEST=atomic_markable_ptr_test
BUFFER_TEST=buffer_test
MEMCASEMEM_TEST=memcasemem_test
MEMRCHR_TEST=memrchr_test
VARINT_TEST=varint_test
ARENA_TEST=arena_test

OBJS =	skiplist_test.o insert_only_skiplist_test.o atomic_markable_ptr_test.o \
	buffer_test.o string/buffer.o memcasemem_test.o string/memcasemem.o \
	memrchr_test.o string/memrchr.o varint_test.o util/varint.o \
	arena_test.o util/arena.o util/concurrent/arena.o

DEPS:= ${OBJS:%.o=%.d}

all: ${SKIPLIST_TEST} ${INSERT_ONLY_SKIPLIST_TEST} ${ATOMIC_MARKABLE_PTR_TEST} \
	${BUFFER_TEST} ${MEMCASEMEM_TEST} ${MEMRCHR_TEST} ${VARINT_TEST} \
	${ARENA_TEST}

${SKIPLIST_TEST}: skiplist_test.o
	${CC} ${CXXFLAGS} ${LDFLAGS} skiplist_test.o ${LIBS} -o $@

${INSERT_ONLY_SKIPLIST_TEST}: insert_only_skiplist_test.o
	${CC} ${CXXFLAGS} ${LDFLAGS} insert_only_skiplist_test.o ${LIBS} -o $@

${ATOMIC_MARKABLE_PTR_TEST}: atomic_markable_ptr_test.o
	${CC} ${CXXFLAGS} ${LDFLAGS} atomic_markable_ptr_test.o ${LIBS} -o $@

${BUFFER_TEST}: buffer_test.o string/buffer.o
	${CC} ${CXXFLAGS} ${LDFLAGS} buffer_test.o string/buffer.o ${LIBS} -o $@

${MEMCASEMEM_TEST}: memcasemem_test.o string/memcasemem.o
	${CC} ${CXXFLAGS} ${LDFLAGS} memcasemem_test.o string/memcasemem.o ${LIBS} -o $@

${MEMRCHR_TEST}: memrchr_test.o string/memrchr.o
	${CC} ${CXXFLAGS} ${LDFLAGS} memrchr_test.o string/memrchr.o ${LIBS} -o $@

${VARINT_TEST}: varint_test.o util/varint.o string/buffer.o
	${CC} ${CXXFLAGS} ${LDFLAGS} varint_test.o util/varint.o string/buffer.o ${LIBS} -o $@

${ARENA_TEST}: arena_test.o util/arena.o util/concurrent/arena.o
	${CC} ${CXXFLAGS} ${LDFLAGS} arena_test.o util/arena.o util/concurrent/arena.o ${LIBS} -o $@

clean:
	rm -f ${SKIPLIST_TEST} ${INSERT_ONLY_SKIPLIST_TEST} ${ATOMIC_MARKABLE_PTR_TEST} \
	${BUFFER_TEST} ${MEMCASEMEM_TEST} ${MEMRCHR_TEST} ${VARINT_TEST} ${ARENA_TEST} \
	${OBJS} ${DEPS}

${OBJS} ${DEPS} ${SKIPLIST_TEST} ${INSERT_ONLY_SKIPLIST_TEST} ${ATOMIC_MARKABLE_PTR_TEST} \
	${BUFFER_TEST} ${MEMCASEMEM_TEST} ${MEMRCHR_TEST} ${VARINT_TEST} ${ARENA_TEST} : Makefile

.PHONY : all clean

%.d : %.cpp
	${MAKEDEPEND} ${CXXFLAGS} $< -MT ${@:%.d=%.o} > $@

%.o : %.cpp
	${CC} ${CXXFLAGS} -c -o $@ $<

-include ${DEPS}
