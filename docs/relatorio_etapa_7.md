# Relatorio para a Etapa 7 de Compiladores

Made by Ian Kersz Amaral - 2025/1

Recuperação de Erros e Otimização de Código.

Ferramentas utilizadas:
Docker, Ubuntu 22.04, Clang 14.0.0 (como g++), Make 4.3, Bison 3.8.2 e Flex 2.6.4.

Computador usado para os testes:
Ryzen 9 9900x, 32GB RAM

## Recuperação de Erros

Para ter uma recuperação de erros mais robusta, a função de localização oferecida pelo Bison foi utilizada.

Agora, algumas novas regras foram adicionadas para capturar os errors, tal como:
Erro na declaração de variáveis, tanto genericamente como esquecer o valor de inicialização, ou declarar uma variável sem tipo.
Erro em esquecer o tamanho na declaração de vetores.
Erro na declaração de funções, como esquecer o tipo de retorno ou nos parâmetros.
Erro generico dentro de um bloco de comandos.
Erro em somente um comando, como esquecer o ponto e vírgula.
Erro dentro das expressões dentro de comandos de controle, como if, while e do-while.

Caso um erro seja encontrado, o programa irá imprimir uma mensagem de erro e continuar a análise do código, tentando recuperar o máximo possível de erros.
A etapa de analise semantica tambem ocorre, mas a geração de TACs e código assembly não será realizada se erros forem encontrados.

Caso algum erro fatal seja encontrado, como um erro de sintaxe que não pode ser recuperado, o programa irá imprimir uma mensagem de erro e encerrar a execução.

Um arquivo foi criado para verificar se a recuperação de erros está funcionando corretamente, chamado `recovery.txt`, dentro da pasta de testes.
Com isso, o log gerado pelo programa foi:
```
./etapa7 tests/recovery.txt tests/recovery.out
8 Syntax errors found:
Syntax Error at line 4: Error parsing variable declaration, did you forget the initialization value?
Syntax Error at line 6: Error parsing variable declaration, did you forget the initialization value?
Syntax Error at line 9: Error parsing variable declaration, did you forget the type?
Syntax Error at line 12: Error parsing vector size, are you certain it is an integer?
Syntax Error at line 16: Error parsing if condition, is the expression correct?
Syntax Error at line 23: Error parsing command inside function
Syntax Error at line 27: Error parsing command inside function
Syntax Error at line 31: Error parsing function parameters, are they correctly typed?
Lines: 33

3 Semantic Errors found:
Semantic Error at line 20: Undeclared Variable _y
Semantic Error at line 25: Undeclared Variable _a
Semantic Error at line 25: Variable _a is not a vector
```

## Otimização de Código

Para utilizar as otimizações, é necessário invocar o programa com a flag `-O`:
`./etapa7 -O <nome_do_arquivo> <arquivo_executavel_de_saida>`
