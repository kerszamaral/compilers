# Makefile made by Ian Kersz
# This file may be distributed as it doesnt impact the work done.

PROJECT = etapa4
DEFAULT_VERSION = debug

CXX = g++
STD = -std=c++17
CXXFLAGS = -Wall -Wextra -pedantic -Wconversion $(STD)
RELEASE_FLAGS = -O2
DEBUG_FLAGS = -g -DDEBUG
current_dir = /${shell pwd}

LEX = flex

BISON = bison

.PHONY: all
all: $(DEFAULT_VERSION)

.PHONY: release
release: CXXFLAGS += $(RELEASE_FLAGS)
release: target

.PHONY: debug
debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: BISONFLAGS =
debug: target

.PHONY: target
target: $(PROJECT)

.PHONY: run
run: $(PROJECT)
	./$(PROJECT)

OBJS = lex.yy.o main.o symbol.o parser.tab.o ast.o checkers.o
$(PROJECT): $(OBJS)
	$(CXX) $(OBJS) -o $(PROJECT)

ast.hpp: symbol.hpp
parser.tab.hpp: ast.hpp
semantic.hpp: symbol.hpp
checkers.hpp: semantic.hpp ast.hpp

main.o: parser.tab.hpp checkers.hpp
parser.tab.o: CXXFLAGS += -Wno-sign-conversion
%.o: %.cpp %.hpp
	$(CXX) $(CXXFLAGS) $< -c

lex.yy.cpp: scanner.l parser.tab.hpp ast.hpp
	$(LEX) -o lex.yy.cpp scanner.l 

.PHONY: visualize
visualize: BISONFLAGS = -g --html -v
visualize: parser.tab.cpp parser.tab.hpp

parser.tab.cpp parser.tab.hpp: parser.ypp ast.hpp
	$(BISON) -H -k $(BISONFLAGS) parser.ypp
# -H = Generate Header file, -k = generate symbol_name function

.PHONY: clean
clean:
	rm -f $(PROJECT) lex.yy.cpp *.o .docker-build $(PROJECT).tgz *.tab.* *.html *.xml *.gv parser.output

# Automatically generates the .tgz file with the current directory name
.PHONY: tgz
tgz:
	touch $(PROJECT).tgz
	tar cvzf $(PROJECT).tgz --exclude=$(PROJECT).tgz --exclude-vcs-ignores --exclude=.git . || rm $(PROJECT).tgz

# Automatically invokes the command two times to check for differences
DIFF_FILE1 = diff1.txt
DIFF_FILE2 = diff2.txt
DIFF_FILE ?= tests/sample.txt
.PHONY: diff
diff: $(DIFF_FILE) $(PROJECT)
	@echo "Running diff..."
	@echo "First run:"
	@./$(PROJECT) $(DIFF_FILE) $(DIFF_FILE1)
	@echo "Second run:"
	@./$(PROJECT) $(DIFF_FILE1) $(DIFF_FILE2)
	@if diff $(DIFF_FILE1) $(DIFF_FILE2) > /dev/null; then \
		echo "\nDifferences not found!"; \
	else \
		echo "\nDifferences found!!"; \
	fi
	@rm -f $(DIFF_FILE1) $(DIFF_FILE2)

# Docker related commands 
.PHONY: docker
docker: docker-build
	docker run --volume "$(current_dir)":/project --rm -it $(PROJECT) 

.PHONY: docker-run
docker-run: docker-build
	docker run --volume "$(current_dir)":/project --rm -it $(PROJECT) "make run"

.PHONY: docker-connect
docker-connect: docker-build
	docker run --volume "$(current_dir)":/project --rm -it $(PROJECT) bash 

.PHONY: docker-build
docker-build: .docker-build

.docker-build: Dockerfile
	touch .docker-build
	docker build -t $(PROJECT) . || rm .docker-build

.PHONY: versions
versions:
	$(CXX) --version | grep version && $(LEX) --version && $(BISON) --version | grep bison && $(MAKE) --version | grep GNU
