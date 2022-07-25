
# vim: ft=make noexpandtab

BINDIR = bin
OBJDIR = obj

OBJECTS := $(OBJDIR)/src/mname.o $(OBJDIR)/src/mpktdmp.o
OBJECTS_TESTS := \
	$(OBJDIR)/tests/check.o $(OBJDIR)/tests/check_mname.o

MD=mkdir -v -p

CFLAGS := -Wall -Werror -fpic

test_mname: LDFLAGS += $(shell pkg-config --libs check) -L$(BINDIR)/static -lmname
test_mname: CFLAGS += -DCHECK -g -Wall -Werror

mquery: LDFLAGS += -L$(BINDIR)/static -lmname
mquery: CFLAGS += -g -Wall -Werror

all: $(BINDIR)/static/libmname.a $(BINDIR)/shared/libmname.so mquery test_mname

test_mname: $(OBJECTS_TESTS) | $(BINDIR)/static/libmname.a
	$(CC) -o $@ $^ $(LDFLAGS)

mquery: $(OBJDIR)/src/mquery.o $(OBJECTS) | $(BINDIR)/static/libmname.a
	$(CC) -o $@ $^ $(LDFLAGS)

$(BINDIR)/static/libmname.a: $(OBJECTS)
	$(MD) $(dir $@)
	$(AR) rcs $@ $^

$(BINDIR)/shared/libmname.so: $(OBJECTS)
	$(MD) $(dir $@)
	$(CC) -shared -o $@ $^

$(OBJDIR)/%.o: %.c
	$(MD) $(dir $@)
	$(CC) -c -o $@ $(CFLAGS) $<

.PHONY: clean

clean:
	rm -rf $(OBJDIR); \
	rm -f test_mname; \
	rm -rf $(BINDIR); \
	rm -rf doc; \
	rm -f mquery

