# Relatorio para a Etapa 7 de Compiladores

Made by Ian Kersz Amaral - 2025/1

Recuperação de Erros e Otimização de Código.

Ferramentas utilizadas:
Docker, Ubuntu 22.04, Clang 14.0.0 (como g++), Make 4.3, Bison 3.8.2 e Flex 2.6.4.

Computador usado para os testes:
Ryzen 9 9900x, 32GB DDR5

## Recuperação de Erros

Para ter uma recuperação de erros mais robusta, a função de localização oferecida pelo Bison foi utilizada.

Agora, algumas novas regras foram adicionadas para capturar os errors, tal como:

- Erro na declaração de variáveis, tanto genericamente como esquecer o valor de inicialização, ou declarar uma variável sem tipo.
- Erro em esquecer o tamanho na declaração de vetores.
- Erro na declaração de funções, como esquecer o tipo de retorno ou nos parâmetros.
- Erro generico dentro de um bloco de comandos.
- Erro em somente um comando, como esquecer o ponto e vírgula.
- Erro dentro das expressões dentro de comandos de controle, como if, while e do-while.

Caso um erro seja encontrado, o programa irá imprimir uma mensagem de erro e continuar a análise do código, tentando recuperar o máximo possível de erros.
A etapa de analise semantica tambem ocorre, mas a geração de TACs e código assembly não será realizada se erros forem encontrados.

Caso algum erro fatal seja encontrado, como um erro de sintaxe que não pode ser recuperado, o programa irá imprimir uma mensagem de erro e encerrar a execução.

Um arquivo foi criado para verificar se a recuperação de erros está funcionando corretamente, chamado `recovery.txt`, dentro da pasta de testes.
Com isso, o log gerado pelo programa foi:

```bash
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

Com isso, é possível verificar que a recuperação de erros está funcionando corretamente, pois o programa conseguiu identificar e recuperar de diversos erros no código.

## Otimização de Código

Para utilizar as otimizações, é necessário invocar o programa com a flag `-O`:
`./etapa7 -O <nome_do_arquivo> <arquivo_executavel_de_saida>`

Com isso, o programa irá passar a lista de TACs para o otimizador, que irá aplicar as seguintes otimizações:

- Eliminação de constantes: Substituir expressões constantes por seus valores.
- Propagação de constantes: Substituir variáveis que possuem valores constantes por seus valores.
- Peephole Optimization: Combinar TACs adjacentes para reduzir o número de instruções.
- Eliminação de Simbolos Inúteis: Remover simbolos que, devido a otimização, não são mais utilizados.
- Eliminação de instruções mortas: Remover TACs que não são utilizados.

Para testar as otimizações, um arquivo foi criado, chamado `optimize.txt`, dentro da pasta de testes.

Para testar a performance do programa, ele foi executado duas vezes, uma sem otimizações e outra com otimizações, e o tempo de execução foi medido com o comando `time` do Linux.
O log gerado pelo programa foi:

```bash
./etapa7 tests/optimize.txt tests/unoptimize.out
./etapa7 tests/optimize.txt tests/optimize.out -o
time ./tests/unoptimize.out
Result of the loop: 935228928

real    0m3.689s
user    0m3.403s
sys     0m0.001s
time ./tests/optimize.out
Result of the loop: 935228928

real    0m1.361s
user    0m1.254s
sys     0m0.000s
```

Além disso, o otimizar imprime algumas estatísticas sobre o código otimizado, como o número de TACs eliminados, o número de TACs otimizados e o número de TACs restantes.
O log gerado pelo otimizador foi:

```bash
Optimization log:
Optimizer completed after 10 attempts.
Iterations with changes: 9
Constant Folding optimizations applied: 9
Constant Propagation optimizations applied: 9
Peephole Optimization optimizations applied: 0
Unused symbols removed: 14
Dead instruction elimination optimizations applied: 9

Original TAC size: 32 vs Optimized TAC size: 23
Reducing TAC size by 9 instructions.

Original Number of Temps: 12 vs Optimized Number of Temps: 3
Reducing Temps by 9 temps.
```

Com isso, é possível verificar que as otimizações estão funcionando corretamente, pois o tempo de execução do programa foi reduzido de 3.689 segundos para 1.361 segundos, uma redução de aproximadamente 63%. Apesar disso, o teste de otimização não é muito complexo e o objetivo dele é apenas mostrar o uso correto das otimizações.
