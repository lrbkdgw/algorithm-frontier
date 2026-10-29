# Build and demo for the matching-complexity audit harness.
#
#   make          compile the harness and the three example programs
#   make self-test
#   make demo     self-test plus the three end-to-end audits
#   make clean

CXX      ?= g++
CXXFLAGS ?= -O2 -std=c++17 -Wall -Wextra
BIN      := bin

EXAMPLES := $(BIN)/blossom_reference $(BIN)/hopcroft_karp $(BIN)/trivial_scan

.PHONY: all clean self-test demo check

all: $(BIN)/matchaudit $(EXAMPLES)

$(BIN):
	mkdir -p $(BIN)

$(BIN)/matchaudit: matchaudit.cpp | $(BIN)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(BIN)/%: examples/%.cpp | $(BIN)
	$(CXX) $(CXXFLAGS) -o $@ $<

self-test: $(BIN)/matchaudit
	./$(BIN)/matchaudit --self-test

# Three different outcomes, one per example program. The exit status is
# non-zero for the refuted and the incorrect program on purpose, so the recipes
# tolerate it and print the verdicts instead.
demo: all
	@echo "=== control: a truly linear program must not be refuted ==="
	./$(BIN)/matchaudit --solver ./$(BIN)/trivial_scan --families empty \
	  --sizes 1000000,2000000,4000000 --max-n 16000000 || true
	@echo
	@echo "=== correct but classical: expected growth closest to a power law ==="
	./$(BIN)/matchaudit --solver ./$(BIN)/blossom_reference \
	  --families empty,triangle-union,random-regular:3 \
	  --sizes 500,1000,2000 --md report-classical.md || true
	@echo
	@echo "=== narrow scope behind a broad interface: expected rejection ==="
	./$(BIN)/matchaudit --solver ./$(BIN)/hopcroft_karp --crosscheck 40 || true

clean:
	rm -rf $(BIN) report-classical.md
