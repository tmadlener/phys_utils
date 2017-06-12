# check if we are on a heplx and set the path accordingly if so
# this allows me to compile remotely from inside emacs using tramp

# include this in makefiles that should be callable in that way via
# 'include /path/to/make_setup_heplx.mk'

ifneq (,$(findstring oeaw.ac.at,$(HOSTNAME)))
export PATH=$(MAKE_PATH)
export LD_LIBRARY_PATH=$(MAKE_LD_LIBRARY_PATH)
GCC_BASE_DIR=$(MAKE_ROOT_GCC_TOOLCHAIN)/bin/
ROOT_CONFIG_BIN=$(MAKE_ROOT_CONFIG_BIN)
else
GCC_BASE_DIR=
ROOT_CONFIG_BIN=$(shell which root-config)
endif
