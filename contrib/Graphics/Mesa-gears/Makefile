	TARGET  = GEARS
SOURCES = gears.cpp
INSTDIR = $(BINDIR)/APPS


SHAREDIR   = $(MONADIR)/share
ifneq ($(BUILD_TARGET),ELF)
include $(SHAREDIR)/configs/monapi-ex5.inc
else
include $(SHAREDIR)/configs/monapi-el5.inc
endif

INCLUDE += -I$(INCDIR)/monalibc

ADDLINK  = -lOSMesa-imp -lmonalibc-imp
