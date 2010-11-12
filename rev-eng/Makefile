CFLAGS+=-W -O2 -DNDEBUG -Dlinux
TARGETS=omni
FILES_omni=omni probe-node easy-com

.PHONY: all
all: $(TARGETS)

omni: $(patsubst %,%.o,$(FILES_omni))
	$(CC) $(CFLAGS) -lraw1394 -o $@ $^

test-probe: test-probe.c probe-node.c
	$(CC) $(CFLAGS) -lraw1394 -o $@ $^

omni.o: probe-node.h
probe-node.o: probe-node.h

.PHONY: clean
clean:
	-rm -f $(TARGETS) $(patsubst %,%.o,$(FILES_omni))
