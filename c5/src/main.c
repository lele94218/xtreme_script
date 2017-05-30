#include "c.h"

/* current token */
int token;

/* pointer to source code string */
char *src, *old_src;

/* default size of text/data/stack */
int poolsize;

/* line number */
int line;

/* text segment, stack */
int *text, *old_text, *stack;

/* data segment */
char *data;

/* virtual machine registers */
int *pc, *bp, *sp, ax, cycle;

/* instructions */
enum
{
    LEA,
    IMM,
    JMP,
    CALL,
    JZ,
    JNZ,
    ENT,
    ADJ,
    LEV,
    LI,
    LC,
    SI,
    SC,
    PUSH,
    OR,
    XOR,
    AND,
    EQ,
    NE,
    LT,
    GT,
    LE,
    GE,
    SHL,
    SHR,
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    OPEN,
    READ,
    CLOS,
    PRTF,
    MALC,
    MSET,
    MCMP,
    EXIT
};

void next()
{
    token = *src++;
    return;
}

void expression(int level)
{
}

void program()
{
    /* get next token */
    next();
    while (token > 0)
    {
        printf("token is: %c\n", token);
        next();
    }
}

int eval()
{
    int op, *tmp;
    while (1)
    {

        printf("ok");
        op = *pc++;
        printf("%ld %ld\n", op, sp);
        if (op == IMM)
        {
            /* load immediate value to ax */
            ax = *pc++;
        }
        else if (op == LC)
        {
            /* load char to ax, address in ax */
            ax = *(char *)ax;
        }
        else if (op == LI)
        {
            /* load integer to ax, address in ax */
            ax = *(int *)ax;
        }
        else if (op == SC)
        {
            /* save char to address, value in ax, address on stack */
            ax = *(char *)*sp++ = ax;
        }
        else if (op == SI)
        {
            /* save integer to address, value in ax, address on stack */
            *(int *)*sp++ = ax;
        }
        else if (op == PUSH)
        {
            // printf("ok!\n");
            // *(sp - 1l) = 10;
            // printf("ok!\n");
            /* push the value of ax onto the stack */
            *(--sp) = ax;
        }
        else if (op == JMP)
        {
            /* jump to the address */
            pc = (int *)*pc;
        }
        else if (op == JZ)
        {
            /* jump if ax is zero */
            pc = ax ? pc + 1 : (int *)*pc;
        }
        else if (op == JNZ)
        {
            /* jump if ax is not zero */
            pc = ax ? (int *)*pc : pc + 1;
        }
        else if (op == CALL)
        {
            *--sp = (int)(pc + 1);
            pc = (int *)*pc;
        }
        else if (op == ENT)
        {
            /* make new stack frame */
            *--sp = (int)bp;
            bp = sp;
            sp = sp - *pc++;
        }
        else if (op == ADJ)
        {
            /* remove arguments from frame */
            sp = sp + *pc++;
        }
        else if (op == LEV)
        {
            /* restore old call frame */
            sp = bp;
            bp = (int *)*sp++;
            pc = (int *)*sp++;
        }
        else if (op == LEA)
        {
            ax = (int)(bp + *pc++);
        }
        else if (op == OR)
        {
            ax = *sp++ | ax;
        }
        else if (op == XOR)
        {
            ax = *sp++ ^ ax;
        }
        else if (op == AND)
        {
            ax = *sp++ & ax;
        }
        else if (op == EQ)
        {
            ax = *sp++ == ax;
        }
        else if (op == NE)
        {
            ax = *sp++ != ax;
        }
        else if (op == LT)
        {
            ax = *sp++ < ax;
        }
        else if (op == LE)
        {
            ax = *sp++ <= ax;
        }
        else if (op == GT)
        {
            ax = *sp++ > ax;
        }
        else if (op == GE)
        {
            ax = *sp++ >= ax;
        }
        else if (op == SHL)
        {
            ax = *sp++ << ax;
        }
        else if (op == SHR)
        {
            ax = *sp++ >> ax;
        }
        else if (op == ADD)
        {
            ax = *sp++ + ax;
        }
        else if (op == SUB)
        {
            ax = *sp++ - ax;
        }
        else if (op == MUL)
        {
            ax = *sp++ * ax;
        }
        else if (op == DIV)
        {
            ax = *sp++ / ax;
        }
        else if (op == MOD)
        {
            ax = *sp++ % ax;
        }
        else if (op == EXIT)
        {
            printf("exit(%d)", *sp);
            return *sp;
        }
        else if (op == OPEN)
        {
            ax = open((char *)sp[1], sp[0]);
        }
        else if (op == CLOS)
        {
            ax = close(*sp);
        }
        else if (op == READ)
        {
            ax = read(sp[2], (char *)sp[1], *sp);
        }
        else if (op == PRTF)
        {
            tmp = sp + pc[1];
            ax = printf((char *)tmp[-1], tmp[-2], tmp[-3], tmp[-4], tmp[-5], tmp[-6]);
        }
        else if (op == MALC)
        {
            ax = (int)malloc(*sp);
        }
        else if (op == MSET)
        {
            ax = (int)memset((char *)sp[2], sp[1], *sp);
        }
        else if (op == MCMP)
        {
            ax = memcmp((char *)sp[2], (char *)sp[1], *sp);
        }
        else
        {
            printf("unknown instruction:%d\n", op);
            return -1;
        }
    }
    return 0;
}

int main(int argc, char **argv)
{
    int i, fd;
    argc--;
    argv++;

    poolsize = 256 * 1024;
    line = 1;

    if ((fd = open(*argv, 0)) < 0)
    {
        printf("could not open (%s)\n", *argv);
        return -1;
    }

    if (!(src = old_src = malloc(poolsize)))
    {
        printf("could not malloc (%d) for source area\n", poolsize);
        return -1;
    }

    /* read the source file */
    if ((i = read(fd, src, poolsize - 1)) <= 0)
    {
        printf("read() returned %d\n", i);
        return -1;
    }
    /* add EOF character */
    src[i] = 0;
    close(fd);

    /* allocate memory for virtual machine */
    if (!(text = old_text = malloc(poolsize)))
    {
        printf("could not malloc(%d) for text area\n", poolsize);
        return -1;
    }

    if (!(data = malloc(poolsize)))
    {
        printf("could not malloc(%d) for data area\n", poolsize);
        return -1;
    }
    if (!(stack = malloc(poolsize)))
    {
        printf("could not malloc(%d) for stack area\n", poolsize);
        return -1;
    }

    memset(text, 0, poolsize);
    memset(data, 0, poolsize);
    memset(stack, 0, poolsize);

    /* initialize virtual machine registers */
    bp = sp = (int *)((unsigned long)stack + poolsize);
    printf("%ld %ld %ld\n", stack, poolsize, sp);

    i = 0;
    text[i++] = IMM;
    text[i++] = 10;
    text[i++] = PUSH;
    text[i++] = IMM;
    text[i++] = 20;
    text[i++] = ADD;
    text[i++] = PUSH;
    text[i++] = EXIT;
    pc = text;

    program();

    return eval();
}
