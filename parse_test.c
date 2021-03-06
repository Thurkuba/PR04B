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
	//	printf("%d", instr -> opds[0] -> type);
	printf("operand\t= ");
	
	if(instr -> opds[0] -> type == REGISTER)
	  printf("Register(%d)", instr -> opds[0] -> value);
	if(instr -> opds[0] -> type == NUMBER_TYPE)
	  printf("Number(%d)", instr -> opds[0] -> value);
	if(instr -> opds[0] -> type == LABEL)
	  printf("Label(\"%s\")", instr -> opds[0] -> value);
	if(instr -> opds[0] -> type == STRING)
	  printf("String(\"%s\")", instr -> opds[0] -> value);

 
	for(int i = 1; i < 3; i++) {
	  if(instr -> opds[i] != NULL) {
	    	if(instr -> opds[i] -> type == REGISTER)
		  printf(", Register(%d)", instr -> opds[i] -> value);
		if(instr -> opds[i] -> type == NUMBER_TYPE)
		  printf(", Number(%d)", instr -> opds[i] -> value);
		if(instr -> opds[i] -> type == LABEL)
		  printf(", Label(\"%s\")", instr -> opds[i] -> value);
		if(instr -> opds[0] -> type == STRING)
		  printf(", String(\"%s\")", instr -> opds[i] -> value);
	  }	  

	  else break;
	}
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
    
    /* leitura das linhas */
    
    int n = read_line(input, b);
    while ((n = read_line(input, b)) != 0) {
      buffer = (char*) b -> data;
      if (parse(buffer, st, &instr, &errptr))
	print_instr(instr, buffer);
      else
	printf("%s\n", errptr);
   
    }
    
    /* TESTE PRINT_INSTR */
    /*
    buffer = estrdup("teste ADD $0, $1, 1\n");
    parse(buffer, st, &instr, &errptr);
    print_instr(instr, buffer);
    return 0;
    */
    /*
    Operand **opds = emalloc(sizeof((Operand*)*3));
    opds[0] = operand_create_register(0);
    opds[1] = operand_create_register(1);
    opds[2] = operand_create_number(2);

    instr = malloc(sizeof(Instruction));
    instr -> label = "labelblabla";
    instr -> op = malloc(sizeof(Operator));
    instr -> op = "ADD";
    instr -> &opds = &opds;
    print_instr(instr, "seila");
    */
}
