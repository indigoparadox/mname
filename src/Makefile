
# vim: ft=make noexpandtab

OBJECTS := mname.o

BINDIR_LINUX = ../bin/linux
BINDIR_MSP430G2553 = ../bin/msp430g2553

OBJDIR_TESTS = ../obj/test_mname

MD=mkdir -v -p

CFLAGS := -Wall -Werror -I../../mfat/src

$(BINDIR_LINUX)/libmname.a: OBJDIR := ../obj/linux

$(BINDIR_MSP430G2553)/libmname.a: OBJDIR := ../obj/msp430g2553
$(BINDIR_MSP430G2553)/libmname.a: AR := msp430-ar
$(BINDIR_MSP430G2553)/libmname.a: CC := msp430-gcc
$(BINDIR_MSP430G2553)/libmname.a: CFLAGS += \
	-mmcu=msp430g2553 \
	-g \
	-pg \
	-Os \
	-DUSE_DISK_RO \
	-fno-builtin \
	-ffreestanding \
	-nostdlib

test_mname: LDFLAGS += $(shell pkg-config --libs check) -L../bin/linux/ -lmname
test_mname: CFLAGS += -DCHECK -g -Wall -Werror
test_mname: OBJDIR := ../obj/test_mname

mquery: LDFLAGS += -L../bin/linux/ -lmname
mquery: CFLAGS += -g -Wall -Werror
mquery: OBJDIR := ../obj/mquery

all: $(BINDIR_LINUX)/libmname.a $(BINDIR_MSP430G2553)/libmname.a mquery

test_mname: $(BINDIR_LINUX)/libmname.a $(OBJDIR_TESTS)/check.o $(OBJDIR_TESTS)/check_mname.o
	$(CC) -o $@ $^ $(LDFLAGS)

mquery: $(BINDIR_LINUX)/libmname.a ../obj/linux/mquery.o
	$(CC) -o $@ $^ $(LDFLAGS)

$(BINDIR_LINUX)/libmname.a: $(addprefix ../obj/linux/,$(OBJECTS))
	$(MD) $(BINDIR_LINUX)
	$(AR) rcs $@ $^

$(BINDIR_MSP430G2553)/libmname.a: $(addprefix ../obj/msp430g2553/,$(OBJECTS))
	$(MD) $(BINDIR_MSP430G2553)
	$(AR) rcs $@ $^

../obj/linux/%.o: $(notdir %.c)
	$(MD) $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJDIR_TESTS)/%.o: ../tests/$(notdir %.c)
	$(MD) $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS)

../obj/msp430g2553/%.o: $(notdir %.c)
	$(MD) $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean

clean:
	rm -rf ../obj; \
	rm -f test_mname; \
	rm -rf ../bin

