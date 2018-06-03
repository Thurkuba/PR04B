#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "error.h"
#include "buffer.h"
#include "queue.h"
#include "optable.h"
#include "parser.h"

/**
 * print_instr - prints instruction
 * @instr: instruction struct pointer
 */
void
print_instr(Instruction *instr, const char *line) {
    if (!instr) printf("instr é NULL!\n");
    else {
        /* line */
        printf("line\t= %s", line);
        /* label */
        if (instr -> label)
            printf("label\t= %s\n", instr -> label);
        else
            printf("label\t= n/a\n");
        /* operator */
        printf("op\t= %s\n", instr -> op -> name);
        /* operands */
        /* PERCORRER O VETOR DE OPERANDOS E IMPRIMIR */
        printf("\n");
    }
}

int main(int argc, char *argv[]) {
    /* declaração e inicialização */
    Buffer          *b = buffer_create(sizeof(char));
    SymbolTable     st = stable_create();
    const char      *errptr = NULL, *buffer;
    Instruction     *instr;
    FILE            *input = fopen(argv[1], "r");
    
    /* null check */
    if (!input) {
        fprintf(stderr, "missing file argument!\n");
        exit(1);
    }
    
//    /* leitura das linhas */
//    int n = read_line(input, b);
//    while ((n = read_line(input, b)) != 0) {
//        buffer = (char*) b -> data;
//        if (parse(buffer, st, &instr, &errptr))
//            print_instr(instr, buffer);
//        else
//            printf("%s\n", errptr);
//    }
    
    /* TESTE PRINT_INSTR */
    
    buffer = estrdup("teste ADD $0, $1, 1\n");
    parse(buffer, st, &instr, &errptr);
    print_instr(instr, buffer);
    return 0;
}
