CXX ?= g++

CXXFLAGS = -Wall -Wextra -pedantic -std=c++11 \
		   -O3 -g \
		   -I../include

## define programs
PROGS ?=

BINDIR ?= bin

BINS = $(addprefix $(BINDIR)/,$(PROGS))
OBJS = $(addsuffix .o,$(PROGS))
DEPS = $(addsuffix .d,$(PROGS))


default: $(BINS)


define link_rule
$(BINDIR)/$1: $1.o | $(BINDIR)
	$$(CXX) $$< -o $$@ $$(CXXFLAGS) $$(LDFLAGS)
endef
$(foreach P,$(PROGS),$(eval $(call link_rule,$(P))))

.cpp.o:
	$(CXX) $< -c -o $@ $(CXXFLAGS) -MP -MMD

$(BINDIR):
	@mkdir -p $@


clean:
	rm -f $(OBJS) $(DEPS) $(BINS) *~
	rm -rf $(BINDIR)

.PHONY: clean

-include $(DEPS)

