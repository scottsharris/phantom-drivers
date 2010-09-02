# FW_METHOD defines the method
FW_METHOD?=libraw1394

CFLAGS?=-Wall -O2
CFLAGS+=-DUSE_$(FW_METHOD)

FILES:= lp-com.cpp base-device.cpp phantom.cpp

ifeq ($(FW_METHOD),libraw1394)
  FILES+=lp-com-libraw1394.cpp
else
  $(error Unknown value for FW_METHOD: $(FW_METHOD). Recognised values are libraw1394 (default))
endif

all: bin/libphantom.a

include extra-commands.mk

bin/libphantom.a: $(addprefix build_dir/,$(call ConvertToObjectNames,$(FILES))) | bin
	$(AR) rcs $@ $^

install: bin/libphantom.a
	@echo "Install libphantom!"

clean:
	rm -rf build_dir bin

$(eval $(call CreateCompileTargets,$(FILES)))
