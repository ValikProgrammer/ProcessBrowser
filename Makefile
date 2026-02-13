PROJECT_NAME := $(notdir $(CURDIR))

SRCDIR := src
OBJDIR := build
DEPDIR := deps
BINDIR := bin
TESTDIR := tests

TARGET ?= $(PROJECT_NAME)

CC ?= gcc
CFLAGS ?= -std=gnu17 -Wall -Wextra -Wpedantic -Isrc/include -g
LDFLAGS ?=

SRC := $(shell find $(SRCDIR) -name '*.c' -print)
OBJ := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRC))
DEPS := $(patsubst $(SRCDIR)/%.c,$(DEPDIR)/%.d,$(SRC))

# Test executables
TEST_SORT := $(TESTDIR)/test_sort
TEST_KILL := $(TESTDIR)/test_kill

.PHONY: all dirs clean distclean check format test test-unit test-integration test-docker

LDFLAGS += -lncurses

all: $(BINDIR)/$(TARGET)

dirs:
	@mkdir -p $(OBJDIR) $(DEPDIR) $(BINDIR)

$(BINDIR)/$(TARGET): $(OBJ) | dirs
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | dirs
	$(CC) $(CFLAGS) -MMD -MP -MF $(DEPDIR)/$*.d -c $< -o $@

-include $(DEPS)

clean:
	rm -rf $(OBJDIR) $(DEPDIR) $(BINDIR)
	rm -f $(TEST_SORT) $(TEST_KILL)

distclean: clean
	@echo "distclean kept just source files"

# Build unit test for sorting
$(TEST_SORT): $(TESTDIR)/test_sort.c $(SRCDIR)/sort.c
	@mkdir -p $(TESTDIR)
	$(CC) $(CFLAGS) -o $@ $^

# Build integration test for killing
$(TEST_KILL): $(TESTDIR)/test_kill.c
	@mkdir -p $(TESTDIR)
	$(CC) $(CFLAGS) -o $@ $<

# Run unit tests
test-unit: $(TEST_SORT)
	@echo "Running unit tests..."
	@./$(TEST_SORT)

# Run integration tests
test-integration: $(TEST_KILL)
	@echo "Running integration tests..."
	@./$(TEST_KILL)

# Run all tests locally
test: test-unit test-integration
	@echo ""
	@echo "All tests passed successfully!"

# Run tests in Docker
test-docker:
	@echo "Building and running tests in Docker..."
	docker build -t $(PROJECT_NAME)-test .
	docker run --rm $(PROJECT_NAME)-test

check:
	@printf "TODO: add test runner for %s\n" $(TARGET)

format:
	@printf "clang-format support not configured yet\n"
