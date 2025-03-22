PROJECT = etapa1

CXX = g++
STD = -std=c++11
CXXFLAGS = -Wall -Wextra -pedantic $(STD)
RELEASE_FLAGS = -O2
DEBUG_FLAGS = -g -DDEBUG
current_dir = /${shell PWD}
DEP_FILES = scanner.h scanner.cpp tokens.h main.cpp

LEX = flex

BISON = bison

.PHONY: all
all: target

.PHONY: target
target: $(PROJECT)

.PHONY:
run: $(PROJECT)
	./$(PROJECT)

$(PROJECT): lex.yy.o
	$(CXX) lex.yy.o -o $(PROJECT)

%.o: %.cpp 
	$(CXX) $(CXXFLAGS) $< -c

lex.yy.cpp: scanner.l $(DEP_FILES)
	$(LEX) -o lex.yy.cpp scanner.l 

.PHONY: docker
docker: docker-build
	docker run --volume "$(current_dir)":/project -it $(PROJECT)

.PHONY: docker-run
docker-run: docker-build
	docker run --volume "$(current_dir)":/project -it $(PROJECT) "make run"

.PHONY: docker-connect
docker-connect: docker-build
	docker run --volume "$(current_dir)":/project -it $(PROJECT) bash 

.PHONY: docker-build
docker-build: .docker-build

.docker-build: Dockerfile
	touch .docker-build
	docker build -t $(PROJECT) .

.PHONY: versions
versions:
	$(CXX) --version | grep version && $(LEX) --version && $(BISON) --version | grep bison && $(MAKE) --version | grep GNU

.PHONY: clean
clean:
	rm -f $(PROJECT) lex.yy.cpp *.o .docker-build $(PROJECT).tgz

.PHONY: tgz
tgz: clean
	tar cvzf $(PROJECT).tgz --exclude .git *