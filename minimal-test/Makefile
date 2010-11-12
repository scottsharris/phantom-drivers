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
	@./proprietary > proprietary.log 2> /dev/null
	@-if [ -z "`grep "unexpected" < proprietary.log`" ]; then \
	  ./opensource > opensource.log; \
	  diff -u proprietary.log opensource.log > comparison.diff; \
	else \
	  grep "unexpected" < proprietary.log;\
	fi
	@echo "Done testing"

.PHONY: clean
clean:
	-rm -f $(TARGETS) $(patsubst %,%.log,$(TARGETS)) comparison.diff
