# FW_METHOD defines the method
FW_METHOD?=libraw1394

CFLAGS?=-Wall -O0 -g
CFLAGS+=-DUSE_$(FW_METHOD)

FILES:= lp-com.cpp base-device.cpp phantom.cpp
TEST_APPS:= config_rom phantom_find

ifeq ($(FW_METHOD),libraw1394)
  FILES+=lp-com-libraw1394.cpp
  LIBS+=-lraw1394
else
ifeq ($(FW_METHOD),macosx)
  FILES+=lp-com-macosx.cpp
  LIBS+=-framework Corefoundation -framework IOKit
else
  $(error Unknown value for FW_METHOD: $(FW_METHOD). Recognised values are libraw1394 (default) and macosx)
endif
endif

all: bin/libphantom.a

include extra-commands.mk

bin/libphantom.a: $(addprefix build_dir/,$(call ConvertToObjectNames,$(FILES))) | bin
	$(AR) rcs $@ $^

install: bin/libphantom.a
	@echo "Install libphantom!"

.PHONY: tests
tests: bin/libphantom.a $(addprefix build_dir/,$(TEST_APPS))
	$(call RunTests,$(TEST_APPS))

clean:
	rm -rf build_dir bin

$(eval $(call CreateCompileTargets,$(FILES)))
$(eval $(call CreateTestAppTargets,$(TEST_APPS)))
