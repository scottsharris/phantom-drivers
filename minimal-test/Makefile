CFLAGS+=-W -O2 -DNDEBUG -Dlinux
LIBS = -lHDU -lHD -lrt

TARGETS=proprietary opensource

.PHONY: all
all: $(TARGETS)

proprietary: proprietary.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

opensource: opensource.c
	$(CC) $(CFLAGS) -lraw1394 -o $@ $<

.PHONY: test
test: $(TARGETS)
	./proprietary > proprietary.log 2> /dev/null
	./opensource > opensource.log
	diff -u proprietary.log opensource.log > comparison.diff

.PHONY: clean
clean:
	-rm -f $(TARGETS) $(patsubst %,%.log,$(TARGETS)) comparison.diff
