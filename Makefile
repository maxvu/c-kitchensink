# Example Makefile
CFLAGS  = -Wall
LDFLAGS =

MAIN   = $(wildcard src/main*.c)
SRCS   = $(wildcard src/*.c)
OBJRLS = $(patsubst src/%.c,build/obj/rls/%.o,$(SRCS))
OBJDBG = $(patsubst src/%.c,build/obj/dbg/%.o,$(SRCS))
OBJASA = $(patsubst src/%.c,build/obj/asa/%.o,$(SRCS))

# various directories
3p/ :
	mkdir $@
bin/ :
	mkdir $@
build/ :
	mkdir $@
build/3p/ : | build/
	mkdir $@
build/obj/ : | build/
	mkdir $@
build/obj/rls/ : | build/obj/
	mkdir $@
build/obj/dbg/ : | build/obj/
	mkdir $@
build/obj/asa/ : | build/obj/
	mkdir $@

# objects
build/obj/rls/%.o : CFLAGS += -Iinclude/ -O3 -s
build/obj/rls/%.o : src/%.c | build/obj/rls/
	$(CC) -c $(CFLAGS) $^ -o $@
build/obj/dbg/%.o : CFLAGS += -Iinclude/ -g -Og
build/obj/dbg/%.o : src/%.c | build/obj/dbg/
	$(CC) -c $(CFLAGS) $^ -o $@
build/obj/asa/%.o : CFLAGS += -Iinclude/ -g -fsanitize=address
build/obj/asa/%.o : src/%.c | build/obj/asa/
	$(CC) -c $(CFLAGS) $^ -o $@
.PHONY: objrls objdbg objasa
objrls : $(OBJRLS)
objdbg : $(OBJDBG)
objasa : $(OBJASA)

# # binaries, suffixed
bin/myproject.rls : LDFLAGS += -O3 -s
bin/myproject.rls : $(OBJRLS) build/obj/rls/main.o | bin/
	$(CC) $^ -o $@ $(LDFLAGS)
bin/myproject.dbg : $(OBJDBG) $(DEPS) build/obj/dbg/main.o | bin/
	$(CC) $^ -o $@ $(LDFLAGS)
bin/myproject.asa : LDFLAGS += -fsanitize=address
bin/myproject.asa : $(OBJASA) $(DEPS) build/obj/asa/main.o | bin/
	$(CC) $^ -o $@ $(LDFLAGS)

.PHONY : clean
clean:
	rm -rf bin/ build/

