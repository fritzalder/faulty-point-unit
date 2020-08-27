# Copyright (c) Open Enclave SDK contributors.
# Licensed under the MIT License.

include ../../openenclave-master/samples/config.mk

ifeq ($(LVI_MITIGATION), ControlFlow)
	ifeq ($(LVI_MITIGATION_BINDIR),)
		$(error LVI_MITIGATION_BINDIR is not set)
	endif
	# Only run once.
	ifeq (,$(findstring $(LVI_MITIGATION_BINDIR),$(CC)))
		CC := $(LVI_MITIGATION_BINDIR)/$(CC)
	endif
	COMPILER := $(COMPILER)-lvi-cfg
endif

CFLAGS=$(shell pkg-config oeenclave-$(COMPILER) --cflags)
LDFLAGS=$(shell pkg-config oeenclave-$(COMPILER) --libs)
INCDIR=$(shell pkg-config oeenclave-$(COMPILER) --variable=includedir)

# Workaround a version-specific EDL import so that the PoC can be ran on both
# vulnerable and patched OE SDK versions; see also:
# https://github.com/openenclave/openenclave/commit/8c5ca84beded89133e99f75c5e9a9556b69124a8
OE_VERSION=$(shell pkg-config oeenclave-$(COMPILER) --modversion)
ifeq ($(OE_VERSION), 0.9.0)
	SYSTEM_EDL = "enclave{\n};"
else
	SYSTEM_EDL = 'enclave {\nfrom "openenclave/edl/logging.edl" import *;\nfrom "openenclave/edl/syscall.edl" import *;\nfrom "platform.edl" import *;\n};'
endif

all:
	$(MAKE) build
	$(MAKE) keys
	$(MAKE) sign

build:
	@ echo "Compilers used: $(CC), $(CXX)"
	echo $(SYSTEM_EDL) > ../system.edl
	oeedger8r ../helloworld.edl --trusted \
                --search-path .. \
                --search-path $(INCDIR) \
                --search-path $(INCDIR)/openenclave/edl/sgx
	$(CC) -g -c $(CFLAGS) -DOE_API_VERSION=2 enc.c -o enc.o
	$(CC) -g -c $(CFLAGS) -DOE_API_VERSION=2 helloworld_t.c -o helloworld_t.o
	$(CC) -o helloworldenc helloworld_t.o enc.o $(LDFLAGS)

sign:
	oesign sign -e helloworldenc -c helloworld.conf -k private.pem

clean:
	rm -f enc.o helloworldenc helloworldenc.signed private.pem public.pem helloworld_t.o helloworld_t.h helloworld_t.c helloworld_args.h

keys:
	openssl genrsa -out private.pem -3 3072
	openssl rsa -in private.pem -pubout -out public.pem
