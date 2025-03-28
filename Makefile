# Makefile made by Ian Kersz
# This file may be distributed as it doesnt impact the work done.

PROJECT = etapa2
DEFAULT_VERSION = debug

CXX = g++
STD = -std=c++11
CXXFLAGS = -Wall -Wextra -pedantic -Wconversion $(STD)
RELEASE_FLAGS = -O2
DEBUG_FLAGS = -g -DDEBUG
current_dir = /${shell pwd}
DEP_FILES = tokens.h scanner.h

LEX = flex

BISON = bison

.PHONY: all
all: $(DEFAULT_VERSION)

.PHONY: release
release: CXXFLAGS += $(RELEASE_FLAGS)
release: target

.PHONY: debug
debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: target

.PHONY: target
target: $(PROJECT)

.PHONY:
run: $(PROJECT)
	./$(PROJECT)

OBJS = lex.yy.o main.o scanner.o
$(PROJECT): $(OBJS)
	$(CXX) $(OBJS) -o $(PROJECT)

%.o: %.cpp %.hpp
	$(CXX) $(CXXFLAGS) $< -c

lex.yy.cpp: scanner.l $(DEP_FILES)
	$(LEX) -o lex.yy.cpp scanner.l 

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

.PHONY: clean
clean:
	rm -f $(PROJECT) lex.yy.cpp *.o .docker-build $(PROJECT).tgz

.PHONY: tgz
tgz:
	touch $(PROJECT).tgz
	tar cvzf $(PROJECT).tgz --exclude=$(PROJECT).tgz --exclude-vcs-ignores --exclude=.git . || rm $(PROJECT).tgz
