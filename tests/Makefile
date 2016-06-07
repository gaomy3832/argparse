PROGS = \
	   ArgValueTest \
	   ArgumentParserTest \

BINDIR = bin

default: test

include ../makefile.inc


test: $(BINS)
	@for b in $(BINS); do $$b || exit; done


# default gtest path is under the test directory.
GTEST ?= googletest
GTESTVER = 1.7.0

CXXFLAGS += -I$(GTEST)/include
LDFLAGS += $(GTEST)/make/gtest_main.a -lpthread

$(OBJS): $(GTEST)

$(BINS): $(GTEST)


$(GTEST):
	@wget https://github.com/google/googletest/archive/release-$(GTESTVER).tar.gz; \
		tar zxvf release-$(GTESTVER).tar.gz && rm release-$(GTESTVER).tar.gz; \
		mv googletest-release-$(GTESTVER) $(GTEST); \
		cd $(GTEST); cd make; make

clean_gtest:
	rm -rf $(GTEST)

.PHONY: clean_gtest
