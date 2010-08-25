CFLAGS+=-W -O2 -DNDEBUG -Dlinux
TARGETS=omni

.PHONY: all
all: $(TARGETS)

omni: omni.c
	$(CC) $(CFLAGS) -lraw1394 -o $@ $<

.PHONY: clean
clean:
	-rm -f $(TARGETS)
