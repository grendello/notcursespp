OS_NAME         := $(shell uname)

ifeq ($(OS_NAME),Linux)
USE_CLANG       ?= no

ifneq ($(USE_CLANG),no)
COMPILER        ?= $(shell which clang++-9 2> /dev/null || true)
COMPILER        ?= $(shell which clang++-8 2> /dev/null || true)
COMPILER        ?= $(shell which clang++ 2> /dev/null || true)
endif

COMPILER        ?= $(shell which g++-9 2> /dev/null || true)
COMPILER        ?= $(shell which g++ 2> /dev/null || true)
COMPILER        ?= $(shell which c++ 2> /dev/null || true)
COMPILER        ?= $(CXX)

CONCURRENCY     := $(shell nproc)
else
COMPILER        ?= $(shell which clang++ 2> /dev/null || true)
COMPILER        ?= $(shell which c++ 2> /dev/null || true)
COMPILER        ?= $(CXX)

CONCURRENCY     := $(shell sysctl -n hw.ncpu)
endif

BUILD_DIR       := _builds
BIN_DIR         := $(BUILD_DIR)/bin
BUILD_DIR_STAMP := $(BUILD_DIR)/.stamp
NINJA_SCRIPT    := $(BUILD_DIR)/build.ninja
CONFIGURATION   := DEBUG
OPTIMIZE_NATIVE := yes
PROFILING       := no
NINJA_OPTIONS   := -j$(CONCURRENCY)
V               :=
PREFIX          := /usr/local
CMAKE_OPTIONS   := -GNinja

PROJECT_NAME    = notcurses++

RUNNABLES       = demo
ARGS            :=

ifneq ($(COMPILER),)
CMAKE_OPTIONS += -DCMAKE_CXX_COMPILER="$(COMPILER)"
endif

ifneq ($(PREFIX),)
CMAKE_OPTIONS += -DCMAKE_INSTALL_PREFIX="$(PREFIX)"
endif

ifneq ($(V),)
NINJA_OPTIONS += -v
endif

ifeq ($(CONFIGURATION),DEBUG)
CMAKE_OPTIONS += -DCMAKE_BUILD_TYPE=Debug
else
CMAKE_OPTIONS += -DCMAKE_BUILD_TYPE=Release
endif

ifneq ($(OPTIMIZE_NATIVE),no)
CMAKE_OPTIONS += -DOPTIMIZE_NATIVE=ON
endif

ifneq ($(PROFILING),no)
CMAKE_OPTIONS += -DENABLE_PROFILING=ON
endif

ifeq ($(shell uname),Darwin)
BOOST_INCLUDEDIR = /usr/local/include
BOOST_LIBRARYDIR = /usr/local/lib

export BOOST_INCLUDEDIR
export BOOST_LIBRARYDIR
endif

all: prepare build

prepare: $(BUILD_DIR_STAMP)

$(BUILD_DIR_STAMP): Makefile
	install -d -m 755 $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake $(CMAKE_OPTIONS) ..
	touch $@

build:
	ninja $(NINJA_OPTIONS) -C $(BUILD_DIR)

install:
	DESTDIR=$(DESTDIR) ninja $(NINJA_OPTIONS) -C $(BUILD_DIR) install

clean:
	rm -rf $(BUILD_DIR)

# $(1) - runnable
define RunTemplate
$(BIN_DIR)/$(1): $(BUILD_DIR_STAMP)
	ninja $(NINJA_OPTIONS) -C $(BUILD_DIR) bin/$(1)

run-$(1): $(BIN_DIR)/$(1)
	$$(BIN_DIR)/$(1) $$(ARGS)
endef

$(foreach runnable,$(RUNNABLES),$(eval $(call RunTemplate,$(runnable))))

.PHONY: build
