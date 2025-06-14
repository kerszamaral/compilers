# Relatorio para a Etapa 6 de Compiladores

Made by Ian Kersz Amaral - 2025/1

Geração de Código Assembly.

Ferramentas utilizadas:
Docker, Ubuntu 22.04, Clang 14.0.0 (como g++), Make 4.3, Bison 3.8.2 e Flex 2.6.4.

Para gerar o código assembly o qual foi usado como ponto de partida para a implementação,
foi compilado varios códigos pequenos utilizando o Clang.

A compilação do código pode ser feita através do comando:
`g++ -masm=intel -arch x86_64 -Wno-unused-command-line-argument <nome_do_arquivo>.S -o <arquivo de saida>`

Apesar disso, o proprio programa já gera o código assembly e também faz a compilação do executavel.

Após executar "make" para compilar o código, é possível executar o programa com o comando:
`./etapa6 <nome_do_arquivo> <arquivo_executavel_de_saida>`

O programa irá gerar o código assembly no arquivo `<arquivo_executavel_de_saida>.S` e também compilará o executável.

Diversos códigos, disponíveis no diretório "testes", foram testados e o código assembly gerado foi verificado quanto à sua correção.

Os testes foram:
bool.txt -> Testa os operadores lógicos e condicionais. As operações de AND, OR e NOT são mais complexas, mas as saidas estão corretas.
control.txt -> Testa os comandos de controle de fluxo, como if, while e for.
fact.txt -> Testa a função fatorial recursiva, fazendo uso de uma pilha global.
math.txt -> Testa as operações matemáticas, como soma, subtração, multiplicação, divisão e modulo para todos os tipos de dados.
read.txt -> Testa a leitura de dados do usuário.
reverse.txt -> Testa as operações em vetores.
sample.txt -> É o arquivo de exemplo fornecido, que testa diversas funcionalidades da linguagem.
vec.txt -> Teste mais autocontido de vetores.

Segue os logs de execução dos testes, anotados com o nome do arquivo de teste:
bool.txt:

```bash
./etapa6 tests/bool.txt bool
./bool
Initial Ints: a 15 b 16
Less: 1
Greater: 0
Less or Equal: 1
Greater or Equal: 0
Equal: 0
Not Equal: 1
Logical AND: 1
Logical OR: 1
Logical NOT: 0
Initial byte: c  d
Less: 1
Greater: 0
Less or Equal: 1
Greater or Equal: 0
Equal: 0
Not Equal: 1
Logical AND: 1
Logical OR: 1
Logical NOT: 0
Initial Reals: e 15.000000 f 1.000000
Less: 0
Greater: 1
Less or Equal: 0
Greater or Equal: 1
Equal: 0
Not Equal: 1
Logical AND: 0
Logical OR: 0
Logical NOT: 1
```

control.txt:

```bash
./etapa6 tests/control.txt control
./control
Searching for the first even number > 10
Checking index 0, value is 5.
Checking index 1, value is 8.
Checking index 2, value is 12.
Found a match!
Search successful. Found value: 12 at index 2.
```

fact.txt:

```bash
./etapa6 tests/fact.txt fact
./fact
Calculating factorial of 10...
The result is: 3628800
```

math.txt:

```bash
./etapa6 tests/math.txt math
./math
Initial int: 15
iAdd: 16
iSub: 15
iMul: 30
iDiv: 15
iMod: 3
Initial byte: ?
cAdd: @
cSub: ?
cMul: ~
cDiv: ?
cMod:
Initial Real: 15.000000
fAdd: 16.000000
fSub: 15.000000
fMul: 30.000000
fDiv: 15.000000
```

read.txt:

```bash
./etapa6 tests/read.txt read
./read
Int a before modification: 1
2
Int a after modification: 2
byte b before modification: a
b
byte b after modification: b
real (float) c before modification: 3.000000
4.01
real (float) c after modification: 4.010000
```

reverse.txt:

```bash
./etapa6 tests/reverse.txt reverse
./reverse
Copying array...
Reversing copied array...
Reversed array: 50 40 30 20 10
```

sample.txt:

```bash
./etapa6 tests/sample.txt sample
./sample
01374159024
Digite um numero:
25
...................
Dobrando algumas vezes y fica 512
A era=15
OK!
```

vec.txt:

```bash
./etapa6 tests/vec.txt vec
./vec
Int Before modification: a[4] 4
Int After modification: a[4] 15
15 Greater than 10:
Byte Before modification: d[2] d
Byte After modification: d[2] d
d Greater than 'b':
Real Before modification: g[2] 3.000000
Real After modification: g[2] 10.000000
10.000000 Greater than 0.5:
End of test.
```
