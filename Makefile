
# vim: ft=make noexpandtab

OBJECTS := obj/src/mname.o obj/src/mpktdmp.o
OBJECTS_TESTS := \
	obj/tests/check.o obj/tests/check_mname.o

MD=mkdir -v -p

CFLAGS := -Wall -Werror

BINDIR = bin
OBJDIR = obj

test_mname: LDFLAGS += $(shell pkg-config --libs check) -L$(BINDIR)/static -lmname
test_mname: CFLAGS += -DCHECK -g -Wall -Werror

mquery: LDFLAGS += -L$(BINDIR)/static -lmname
mquery: CFLAGS += -g -Wall -Werror

all: $(BINDIR)/static/libmname.a mquery test_mname

test_mname: $(OBJECTS_TESTS) | $(BINDIR)/static/libmname.a
	$(CC) -o $@ $^ $(LDFLAGS)

mquery: obj/src/mquery.o $(OBJECTS) | $(BINDIR)/static/libmname.a
	$(CC) -o $@ $^ $(LDFLAGS)

$(BINDIR)/static/libmname.a: $(OBJECTS)
	$(MD) $(dir $@)
	$(AR) rcs $@ $^

$(OBJDIR)/%.o: %.c
	$(MD) $(dir $@)
	$(CC) -c -o $@ $(CFLAGS) $<

.PHONY: clean

clean:
	rm -rf $(OBJDIR); \
	rm -f test_mname; \
	rm -rf $(BINDIR)

