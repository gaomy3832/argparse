PROGS = \
	   ArgValueTest \

BINDIR = bin

include ../makefile.inc


test: $(BINS)
	@for b in $(BINS); do $$b || exit; done


# default gtest path is under the top directory.
GTEST_PATH ?= ../googletest/googletest/

CXXFLAGS += -I$(GTEST_PATH)/include
LDFLAGS += $(GTEST_PATH)/make/gtest_main.a -lpthread

