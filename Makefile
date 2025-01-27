# :set noexpandtab

# ╔═╗┬ ┬┌┐┌┌─┐┌┬┐┬┌─┐┌┐┌┌─┐
# ╠╣ │ │││││   │ ││ ││││└─┐
# ╚  └─┘┘└┘└─┘ ┴ ┴└─┘┘└┘└─┘

# Recursive wildcard.
RWILDCARD = $(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call RWILDCARD,$d/,$2))

# binutils/bfd/binary.c, hardcoded "_binary_%s_%s" and ISALNUM().
REDEFINE-SYM = $(shell echo _binary_$1_$3=_binary_$2_$3 | sed 's/[^0-9A-Za-z=]/_/g')

# ╔╦╗┌─┐┬─┐┌─┐┌─┐┌┬┐┌─┐
#  ║ ├─┤├┬┘│ ┬├┤  │ └─┐
#  ╩ ┴ ┴┴└─└─┘└─┘ ┴ └─┘

SOURCES_DIR = $(CURDIR)/sources
BUILD_DIR = $(CURDIR)/build

BINARY = $(BUILD_DIR)/main

CXX_SUFFIX = c
OCL_SUFFIX = cl

CXX_SOURCES = $(call RWILDCARD,$(SOURCES_DIR)/,*.$(CXX_SUFFIX))
OCL_SOURCES = $(call RWILDCARD,$(SOURCES_DIR)/,*.$(OCL_SUFFIX))

CXX_OBJECTS = $(CXX_SOURCES:$(SOURCES_DIR)/%.$(CXX_SUFFIX)=$(BUILD_DIR)/%.o)
OCL_OBJECTS = $(OCL_SOURCES:$(SOURCES_DIR)/%.$(OCL_SUFFIX)=$(BUILD_DIR)/%.$(OCL_SUFFIX).o)

CXX_DEPENDENCIES = $(CXX_OBJECTS:.o=.d)

# ╔═╗┬  ┌─┐┌─┐┌─┐
# ╠╣ │  ├─┤│ ┬└─┐
# ╚  ┴─┘┴ ┴└─┘└─┘

CXX = clang-19

# TODO: See OpenSSF
# -pedantic
CXX_FLAGS = \
	-Wall \
	-Wextra \
	-Wconversion \
	-Werror \
	-std=c23 -O0

CXX_INCLUDE = -iquote $(SOURCES_DIR)

CXX_PREPROCESSOR = -MMD -MP -MT $@ -MF $(@:.o=.d)

LD_FLAGS = -z noexecstack -lOpenCL

# ╔╗ ┬ ┬┬ ┬  ┌┬┐
# ╠╩╗│ ││ │   ││
# ╚═╝└─┘┴ ┴─┘╶┴┘

.PHONY: all

all: $(BINARY)
	@echo "-->" ./$(<:$(CURDIR)/%=%)

$(BINARY): $(CXX_OBJECTS) $(OCL_OBJECTS)
	@echo Generating Code...
	@$(CXX) $^ -o $@ $(LD_FLAGS)

$(CXX_OBJECTS): $(BUILD_DIR)/%.o: $(SOURCES_DIR)/%.$(CXX_SUFFIX)
	@mkdir -p $(dir $@)
	@echo $(<:$(SOURCES_DIR)/%=%)
	@$(CXX) -c $< -o $@ $(CXX_FLAGS) $(CXX_INCLUDE) $(CXX_PREPROCESSOR)

$(OCL_OBJECTS): $(BUILD_DIR)/%.$(OCL_SUFFIX).o: $(SOURCES_DIR)/%.$(OCL_SUFFIX)
	@mkdir -p $(dir $@)
	@echo $(<:$(SOURCES_DIR)/%=%)
	@objcopy -I binary -O elf64-x86-64 $< $@ \
		--redefine-sym $(call REDEFINE-SYM,$<,$(<:$(SOURCES_DIR)/%=%),start) \
		--redefine-sym $(call REDEFINE-SYM,$<,$(<:$(SOURCES_DIR)/%=%),end)   \
		--redefine-sym $(call REDEFINE-SYM,$<,$(<:$(SOURCES_DIR)/%=%),size)  \
		--rename-section .data=.rodata,alloc,load,readonly,data,contents

-include $(CXX_DEPENDENCIES)

# ╔═╗┬  ┌─┐┌─┐┌┐┌
# ║  │  ├┤ ├─┤│││
# ╚═╝┴─┘└─┘┴ ┴┘└┘

.PHONY: clean cleanall mrproper

clean:
	@rm -f $(CXX_OBJECTS) $(OCL_OBJECTS)

cleanall: clean
	@rm -f $(CXX_DEPENDENCIES)

mrproper : cleanall
	@rm -f $(BINARY)

# ╦═╗┬ ┬┌┐┌
# ╠╦╝│ ││││
# ╩╚═└─┘┘└┘

.PHONY: crun run

crun: all run

run:
	@$(BINARY)

# ╦  ┬┌┐┌┬┌─┌─┐
# ║  ││││├┴┐└─┐
# ╩═╝┴┘└┘┴ ┴└─┘

# https://www.gnu.org/software/make/manual/make.html#Static-Pattern
# https://stackoverflow.com/questions/4036191/sources-from-subdirectories-in-makefile
# https://www.gnu.org/software/make/manual/html_node/Catalogue-of-Rules.html
# https://stackoverflow.com/questions/410980/include-a-text-file-in-a-c-program-as-a-char
# https://www.linuxjournal.com/content/embedding-file-executable-aka-hello-world-version-5967
# https://github.com/graphitemaster/incbin/blob/main/incbin.h
