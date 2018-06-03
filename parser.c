/*******************************************************************************
 * parser.c
 *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <limits.h>
#include "error.h"
#include "queue.h"
#include "optable.h"
#include "parser.h"

#define MAX_ERROR_LENGTH 99
#define MAX_STR_LENGTH 1024
/* debug */
#define SEG puts("==============================");

/**
 * isNBytes - determines if token is a number that fits in N bytes
 * @n: unsigned int being evaluated
 * @nbyt: number of bytes
 */
static bool
isNBytes(unsigned int n, unsigned int nbyt) {
    unsigned int b = 0;
    /* right shift */
    do b++;
    while (n >>= 1);
    if (b <= nbyt * 8) return true;
    return false;
}

/**
 * NBytes - returns number of bytes a signed int takes
 * @n: int being evaluated
 */
static int
NBytes(int n) {
    unsigned int b = 0;
    /* right shift */
    do b++;
    while (n >>= 1);
    return b;
}

/**
 * isValidHexa - determines if token is a valid hexadecimal digit or not
 * @token: string being evaluated
 */
static bool
isValidHexa(char* token) {
    for (int i = 1; i < (int) strlen(token); i++)
        if (!isxdigit(token[i])) return false;
    return true;
}

/**
 * isDigitString - determines if token is a valid decimal digit or not
 * @token: string being evaluated
 */
static bool
isDigitString(char* token) {
    for (int i = 0; i < (int) strlen(token); i++)
        if (!isdigit(token[i])) return false;
    return true;
}

/**
 * isValidLabel - determines if label is valid or not (boolean)
 * @token: string being evaluated
 *
 */
static bool
isValidLabel(char* token) {
    /* label == token? */
    if (optable_find(token) != NULL) return false;
    /* label begins with valid char? */
    if (!isalpha(token[0]) && token[0] != 95) return false;
    /* checks the rest of the string */
    for (int i = 1; i < (int) strlen(token); i++) {
        /* ignora \n */
        if (token[i] == 10) continue;
        if (!isalnum(token[i]) && token[i] != 95) return false;
    }
    return true;
}

/**
 * numberHandler - handles number opds
 * @token: string being evaluated
 * @opds: operand vector
 * @at: alias hashtable
 * @errptr: dedo duro
 * @s: original line
 * @i: #opd
 * @nbyt: how long this number can be
 */
void
numberHandler(char *token, Operand **opds, SymbolTable at, const char **errptr,
              const char *s, int i, int nbyt) {
    // TODO: numbers bigger than 32bit
    /* decimal */
    if (isDigitString(token) && isNBytes(atoi(token), nbyt))
        opds[i] = operand_create_number(atoi(token));
    
    /* symbol */
    else if (stable_find(at, token) != NULL) {
        // TODO: mais NULL check (opd != NULL, mas não válido)
        Operand *opd = stable_find(at, token) -> opd;
        if (opd && opd -> type == NUMBER_TYPE)
            opds[i] = operand_create_number(opd -> value.num);
    }
    /* hexa */
    else if (token[0] == '#' && isValidHexa(token) &&
             isNBytes(strtol(token + 1, NULL, 16), nbyt))
        opds[i] = operand_create_number(strtol(token + 1, NULL, 16));
    /* error */
    else {
        *errptr = strstr(s, token);
        set_error_msg("Operand %s is not valid\n", token);
        // TODO: terminar exec?
    }
}

/**
 * labelHandler - handles label opds for CALL/EXTERN/JMP functions
 * @token: string being evaluated
 * @opds: operand vector
 * @at: alias hashtable
 * @errptr: dedo duro
 * @s: original line
 * @i: #opd
 */
void
labelHandler(char *token, Operand **opds, SymbolTable at, const char **errptr,
              const char *s, int i) {
    EntryData *ed;
    Operand *opd;
    if (isValidLabel(token) && (ed = stable_find(at, token)) != NULL &&
       (opd = ed -> opd) != NULL) {
        opds[i] = opd;
    }
    else {
        *errptr = strstr(s, token);
        set_error_msg("Label %s is not valid\n", token);
    }
}

/**
 * negativeHandler - handles negative number types
 * @token: string being evaluated
 * @opds: operand vector
 * @at: alias hashtable
 * @errptr: dedo duro
 * @s: original line
 * @i: #opd
 */
void
negativeHandler(char *token, Operand **opds, const char **errptr,
             const char *s, int i) {
    assert(token[0] == '-');
    /* decimal */
    if (isDigitString(token + 1) && isNBytes(atoi(token + 1), 2))
        opds[i] = operand_create_number(atoi(token));
    /* hexa */
    // TODO: negative hexadecimals?
    /* error */
    else {
        *errptr = strstr(s, token);
        set_error_msg("Operand %s is not valid\n", token);
        // TODO: terminar exec?
    }
}

/**
 * regHandler - handles reg opds
 * @token: string being evaluated
 * @opds: operand vector
 * @at: alias hashtable
 * @errptr: dedo duro
 * @s: original line
 * @i: #opd
 */
void
regHandler(char *token, Operand **opds, SymbolTable at, const char **errptr,
             const char *s, int i) {
    EntryData *ed;
    Operand *opd;
    /* reg $X */
    if (token[0] == '$' && isDigitString(token + 1) && isNBytes(atoi(token + 1), 1))
        opds[i] = operand_create_register(atoi(token + 1));
    /* reg especiais */
    else if (strcmp(token, "rA") == 0) opds[i] = operand_create_register(255);
    else if (strcmp(token, "rR") == 0) opds[i] = operand_create_register(254);
    else if (strcmp(token, "rSP") == 0) opds[i] = operand_create_register(253);
    else if (strcmp(token, "rX") == 0) opds[i] = operand_create_register(252);
    else if (strcmp(token, "rY") == 0) opds[i] = operand_create_register(251);
    else if (strcmp(token, "rZ") == 0) opds[i] = operand_create_register(250);
    /* label */
    else if (isValidLabel(token) && (ed = stable_find(at, token)) != NULL &&
        (opd = ed -> opd) != NULL && opd -> type == REGISTER) {
        opds[i] = opd;
    }
    else {
        *errptr = strstr(s, token);
        set_error_msg("Register %s is not valid\n", token);
    }
}

/**
 * opd_constructor -
 * @str: the operands in string form
 * @mem: where the error could be
 * @op: operator
 * @errptr: array of pointers
 */
Operand **
opd_constructor(const char *s, Queue *q, const Operator *op_bkp,
                const char **errptr, SymbolTable at, const char *label) {
    /* declarações e inicializações */
    Operator *op =  emalloc(sizeof(Operator));
                    memcpy(op, op_bkp, sizeof(Operator));
    Operand         **opds = emalloc(sizeof(Operand*) * 3);
    char            *token = NULL;
    
    /* percorrer opd_types do objeto op */
    for (int i = 0; i < 3; i++) {
        if (!queue_isEmpty(q)) {
            token = queue_dequeue(q);
            printf("dequeue <- %sm\n", token);
        }
        else token = NULL;
        
        if (!token && op -> opd_types[i] != OP_NONE) {
            //            TODO: *errptr = strstr(s, token);
            set_error_msg("Expected operand\n");
            return NULL;
        }
        switch (op -> opd_types[i]) {
            case OP_NONE:
                puts("NONE");
                if (token != NULL) {
                    *errptr = strstr(s, token);
                    set_error_msg("Too many operands\n");
                }
                opds[i] = NULL;
                break;
            case BYTE1:
                puts("BYTE1");
                numberHandler(token, opds, at, errptr, s, i, 1);
                break;
            case BYTE2:
                puts("BYTE2");
                numberHandler(token, opds, at, errptr, s, i, 2);
                break;
            case BYTE3:
                puts("BYTE3");
                numberHandler(token, opds, at, errptr, s, i, 3);
                break;
            case TETRABYTE:
                puts("TETRA");
                numberHandler(token, opds, at, errptr, s, i, 4);
                break;
            case LABEL:
                puts("LABEL");
                labelHandler(token, opds, at, errptr, s, i);
                break;
            case REGISTER:
                puts("REG");
                regHandler(token, opds, at, errptr, s, i);
                break;
            case NEG_NUMBER:
                puts("NEG_NUMBER");
                negativeHandler(token, opds, errptr, s, i);
                break;
            case STRING:
                puts("STR");
                opds[i] = operand_create_string(token);
                break;
            case IMMEDIATE:
                puts("IMMEDIATE");
                if (isDigitString(token))
                    numberHandler(token, opds, at, errptr, s, i, 1);
                else
                    regHandler(token, opds, at, errptr, s, i);
                continue;
            case ADDR2:
                puts("ADDR2");
                if (isDigitString(token))
                    numberHandler(token, opds, at, errptr, s, i, 2);
                else if (isValidLabel(token))
                    labelHandler(token, opds, at, errptr, s, i);
                else
                    negativeHandler(token, opds, errptr, s, i);
                break;
            case ADDR3:
                puts("ADDR3");
                if (isDigitString(token))
                    numberHandler(token, opds, at, errptr, s, i, 3);
                else if (isValidLabel(token))
                    labelHandler(token, opds, at, errptr, s, i);
                else
                    negativeHandler(token, opds, errptr, s, i);
                break;
            case TETRABYTE | NEG_NUMBER:
                puts("TETRABYTE | NEG_NUMBER");
                if (token[0] == '-')
                    negativeHandler(token, opds, errptr, s, i);
                else
                    numberHandler(token, opds, at, errptr, s, i, 4);
                break;
            case REGISTER | TETRABYTE | NEG_NUMBER:
                puts("REGISTER | TETRABYTE | NEG_NUMBER");
                puts("IS");
                if (token[0] == '-') {
                    negativeHandler(token, opds, errptr, s, i);
                }
                else if (isDigitString(token)) {
                    numberHandler(token, opds, at, errptr, s, i, 4);
                }
                else {
                    regHandler(token, opds, at, errptr, s, i);
                }
                /* coloca simbolo na st */
                if (label) {
                    EntryData *ed = stable_insert(at, label).data;
                    ed -> opd = opds[i];
                }
                else {
                    *errptr = strstr(s, label);
                    set_error_msg("Missing or invalid label!\n");
                }
                
                break;
        }
    }
    /* debug */
    for (int i = 0; i < 3; i++) {
        if(opds[i]) {
            if (!opds[i] -> value.num)
                printf("opds[%d].type = %d\nopds[%d].value = NULL\n", i,
                       opds[i] -> type, i);
            else
                printf("opds[%d].type = %d\nopds[%d].value = %d\n", i,
                       opds[i] -> type, i, opds[i] -> value.reg);
        }
        else
            printf("opds[%d] = NULL\n", i);
    }
    return opds;
}

/**
 * parse - Return instruction corresponding to assembly language line.
 * @s: line of assembly code.
 * @alias_table: table of aliases used so far.
 *
 * Returns nonzero on success, zero to signal an error. On success,
 * *instr contains the parsed instruction, unless the line was
 * empty. On error, *errptr, if non-NULL, points to the character in s
 * where the error was found.
 */
int
parse(const char *s, SymbolTable alias_table, Instruction **instr,
      const char **errptr) {
    /* declarações e inicializações */
    char            *line = estrdup(s),
                    *op_str,
                    *token = NULL,
                    *label = NULL;
    const Operator  *op = NULL;
    Operand         **opds = NULL;
    Queue           *q = queue_new();
    
    /* checagem NULL arg */
    if (!alias_table || !s) {
        set_error_msg("calls parse with NULL args!");
        return 0;
    }
    
    /* leitura da linha e parse das palavras com strsep */
    while ((token = strsep(&line, " \n\t,;")) != NULL) {
        /* ignora strings vazios */
        if (strcmp(token, "") == 0) continue;
        /* comentários */
        if (token[0] == '*') break;
        /* parse str */
        if (token[0] == '"') {
            token[strlen(token)] = 32;
            char *str_end = strpbrk(line, "\"");
            *str_end = '\0';
            line = str_end;
            printf("enqueue -> %s\n", token + 1);
            queue_enqueue(q, token + 1, strlen(token));
        }
        /* parse outros */
        else {
            printf("enqueue -> %s\n", token);
            queue_enqueue(q, token, strlen(token));
        }
    }
    /* empty line or NOP // procurar NOP? (rótulo em linha NOP) */
    if (queue_isEmpty(q) || strncmp(queue_peek(q), "NOP", 3) == 0)
        return 1;
    
    while (!queue_isEmpty(q)) {
        token = queue_dequeue(q);
        printf("dequeue <- %s\n", token);
        /* check label */
        if (!label && isValidLabel(token))
            label = token;
        /* check op */
        else if ((op = optable_find(token))) {
            op_str = token;
            /* chama construtor opd */
            if (!(opds = opd_constructor(s, q, op, errptr, alias_table, label)))
                return 0;
            if (!label) label = "-";
        }
    }
    /* label não válido */
    if (label && !isValidLabel(label)) {
        printf("label = %s\n", label);
        *errptr = strstr(s, label);
        set_error_msg("Label %s is not valid\n", label);
        return 0;
    }
    /* op não válido */
    if (!op) {
        *errptr = strstr(s, op_str);
        set_error_msg("Operator %s is not valid\n", op_str);
    }

    /* cria instruções */
    *instr = instr_create(label, op, opds);
    return 1;
}
