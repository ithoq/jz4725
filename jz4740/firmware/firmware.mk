
SOURCES	+= $(wildcard $(FIRMWAREDIR)/jz_7131/*.c)
SOURCES	+= $(wildcard $(FIRMWAREDIR)/*.c)
CFLAGS	+= -DFIRMWARE=$(FIRMWARE)
CFLAGS	+= -I$(FIRMWAREDIR)/
CFLAGS	+= -I$(FIRMWAREDIR)/jz_7131
VPATH   += $(FIRMWAREDIR)/
VPATH   += $(FIRMWAREDIR)/jz_7131

