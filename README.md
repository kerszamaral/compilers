# Compilers

Basic Makefile + Dockerfile + .gitignore + base files for the Compilers class.

If you need to change the "Etapa", this can be done by simply changing the `PROJECT` variable inside the Makefile.

To connect to the docker container, use `make docker-connect`.
This will drop you into a shell with all dependencies installed.

From that, you can use the Makefile normally, using `make` to build the project, and then run the executable generated.

The files inside the folder are synced with the original folder, so you can use whatever you desire to edit them, without needing to be directly in the container.

All other files (such as scanner.l, main.cpp, symbol.h, symbol.cpp) are provided blank for user customization. The token.h file is also already provided.

The `PROJECT`.tgz file can be generated automatically by the `make tgz` command. Please, use it while inside the Docker container, as it needs a specific version of `tar`.
