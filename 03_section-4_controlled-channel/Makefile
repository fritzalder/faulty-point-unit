ifeq ($(SGX_SDK),)
    SGX_SDK          = /localhost/packages/linux-sgx/sgxsdk
endif
export SGX_SDK

SGX_MODE ?= HW

ifneq ($(SGX_MODE), HW)
	Urts_Library_Name := sgx_urts_sim
	UAE_SERVICE_NAME := sgx_uae_service_sim
	MODE_NAME = "SIMULATION"
else
	Urts_Library_Name := sgx_urts
	UAE_SERVICE_NAME := sgx_uae_service
	MODE_NAME = "HARDWARE"
endif
export MODE_NAME
export SGX_MODE

ENCLAVE              = Enclave
SUBDIRS              = $(ENCLAVE)

CC                   = gcc
AS                   = gcc
LD                   = gcc

LIB_SUFX = 64

CFLAGS              += -fPIC -fno-stack-protector -fno-builtin -fno-jump-tables \
                       -fno-common -Wno-attributes -g -D_GNU_SOURCE 
			   
INCLUDE              = -I$(SGX_SDK)/include/ -I$(CURDIR)/../lib/
LDFLAGS             += -linc_proxy -l$(Urts_Library_Name) \
                       -l$(UAE_SERVICE_NAME) -pthread $(SUBDIRS:%=-L %) -L$(SGX_SDK)/lib$(LIB_SUFX)/ -lm \
					   -Wl,--whole-archive -L$(CURDIR)/../lib/ -lfpu_lib -Wl,--no-whole-archive

SOURCES              = $(shell ls *.c)
OBJECTS              = $(SOURCES:.c=.o)
OUTPUT               = main

BUILDDIRS            = $(SUBDIRS:%=build-%)
CLEANDIRS            = $(SUBDIRS:%=clean-%)

.SILENT:
all: $(LIB) $(OUTPUT)
	echo "###########################################################################################"
	echo "# Compiled in $(MODE_NAME) mode. To instead compile in hardware mode, make with SGX_MODE=HW #"
	echo "###########################################################################################"

$(LIB):
	$(MAKE) -C ../lib

$(OUTPUT): $(BUILDDIRS) $(OBJECTS)
	echo "$(INDENT)[LD]" $(OBJECTS) $(LIBS) -o $(OUTPUT) 
	echo "$(CFLAGS)"
	$(LD) $(OBJECTS) $(LDFLAGS) -o $(OUTPUT) 

%.o : %.c
	echo "$(INDENT)[CC] " $<
	$(CC) $(CFLAGS) $(INCLUDE) -c $<

%.o : %.S
	echo "$(INDENT)[AS] " $<
	$(AS) $(INCLUDE) -c $< -o $@

clean: $(CLEANDIRS)
	echo "$(INDENT)[RM]" $(OBJECTS) $(OUTPUT)
	rm -f $(OBJECTS) $(OUTPUT)

$(BUILDDIRS):
	echo "$(INDENT)[===] $(@:build-%=%) [===]"
	$(MAKE) -C $(@:build-%=%) INDENT+="$(INDENT_STEP)" M32=$(M32) curr-dir=$(curr-dir)/$(@:build-%=%)

$(CLEANDIRS):
	echo "$(INDENT)[===] $(@:clean-%=%) [===]"
	$(MAKE) clean -C $(@:clean-%=%) INDENT+="$(INDENT_STEP)" curr-dir=$(curr-dir)/$(@:build-%=%)
