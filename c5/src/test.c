#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

typedef enum {
    BAD_TOKEN,
    NUMBER_TOKEN,
    ADD_OPERATOR_TOKEN,
    SUB_OPERATOR_TOKEN,
    MUL_OPERATOR_TOKEN,
    DIV_OPERATOR_TOKEN,
    RIGHT_PAREN_TOKEN,
    LEFT_PAREN_TOKEN,
    END_OF_LINE_TOKEN
} TokenKind;

#define MAX_TOKEN_SIZE (100)

typedef struct
{
    TokenKind kind;
    double value;
    char str[MAX_TOKEN_SIZE];
} Token;

void get_line(char *line);
void get_token(Token *token);

static char *st_line;
static int st_line_pos;

typedef enum {
    INITIAL_STATUS,
    IN_INT_PART_STATUS,
    DOT_STATUS,
    IN_FRAC_PART_STATUS
} LexerStatus;

void get_token(Token *token)
{
    int out_pos = 0;
    LexerStatus status = INITIAL_STATUS;
    char current_char;

    token->kind = BAD_TOKEN;

    while (st_line[st_line_pos] != '\0')
    {
        current_char = st_line[st_line_pos];
        if ((status == IN_INT_PART_STATUS || status == IN_FRAC_PART_STATUS) && !isdigit(current_char) && current_char != '.')
        {
            token->kind = NUMBER_TOKEN;
            sscanf(token->str, "%lf", &token->value);
            return;
        }
        if (isspace(current_char))
        {
            if (current_char == '\n')
            {
                token->kind = END_OF_LINE_TOKEN;
                return;
            }
            st_line_pos++;
            continue;
        }
        if (out_pos >= MAX_TOKEN_SIZE - 1)
        {
            fprintf(stderr, "token too long.\n");
            exit(1);
        }
        token->str[out_pos] = st_line[st_line_pos];
        st_line_pos++;
        out_pos++;
        token->str[out_pos] = '\0';

        if (current_char == '+')
        {
            token->kind = ADD_OPERATOR_TOKEN;
            return;
        }

        else if (current_char == '-')
        {
            token->kind = SUB_OPERATOR_TOKEN;
            return;
        }
        else if (current_char == '*')
        {
            token->kind = MUL_OPERATOR_TOKEN;
            return;
        }
        else if (current_char == '/')
        {
            token->kind = DIV_OPERATOR_TOKEN;
            return;
        }
        else if (current_char == '(')
        {
            token->kind = LEFT_PAREN_TOKEN;
            return;
        }
        else if (current_char == ')')
        {
            token->kind = RIGHT_PAREN_TOKEN;
            return;
        }
        else if (isdigit(current_char))
        {
            if (status == INITIAL_STATUS)
            {
                status = IN_INT_PART_STATUS;
            }
            else if (status == DOT_STATUS)
            {
                status = IN_FRAC_PART_STATUS;
            }
        }
        else if (current_char == '.')
        {
            if (status == IN_INT_PART_STATUS)
            {
                status = DOT_STATUS;
            }
            else
            {
                fprintf(stderr, "syntax error.\n");
                exit(1);
            }
        }
        else
        {
            fprintf(stderr, "bad character(%c).\n", current_char);
            exit(1);
        }
    }
}

void set_line(char *line)
{
    st_line = line;
    st_line_pos = 0;
}

#define LINE_BUF_SIZE (1024)

static Token st_look_ahead_token;
static int st_look_ahead_token_exists;

static void
my_get_token(Token *token)
{
    if (st_look_ahead_token_exists)
    {
        *token = st_look_ahead_token;
        st_look_ahead_token_exists = 0;
    }
    else
    {
        get_token(token);
    }
}

static void
unget_token(Token *token)
{
    st_look_ahead_token = *token;
    st_look_ahead_token_exists = 1;
}

double parse_expression(void);

static double
parse_primary_expression()
{
    Token token;
    double value;
    int minus_flag = 0;
    
    my_get_token(&token);
    if (token.kind == SUB_OPERATOR_TOKEN) {
        minus_flag = 1;
    } else {
        unget_token(&token);
    }

    my_get_token(&token);
    if (token.kind == NUMBER_TOKEN)
    {
        value = token.value;
    }
    else if (token.kind == LEFT_PAREN_TOKEN)
    {
        value = parse_expression();
        my_get_token(&token);
        if (token.kind != RIGHT_PAREN_TOKEN)
        {
            fprintf(stderr, "missing ')' error.\n");
            exit(1);
        }
    }
    else
    {
        unget_token(&token);
    }

    if (minus_flag)
    {
        value = -value;
    }
    return value;
}

static double
parse_term()
{
    double v1;
    double v2;
    Token token;
    v1 = parse_primary_expression();
    for (;;)
    {
        my_get_token(&token);
        if (token.kind != MUL_OPERATOR_TOKEN && token.kind != DIV_OPERATOR_TOKEN)
        {
            unget_token(&token);
            break;
        }
        v2 = parse_primary_expression();
        if (token.kind == MUL_OPERATOR_TOKEN)
        {
            v1 *= v2;
        }
        else if (token.kind == DIV_OPERATOR_TOKEN)
        {
            v1 /= v2;
        }
    }
    return v1;
}

double
parse_expression()
{
    double v1;
    double v2;
    Token token;

    v1 = parse_term();
    for (;;)
    {
        my_get_token(&token);
        if (token.kind != ADD_OPERATOR_TOKEN && token.kind != SUB_OPERATOR_TOKEN)
        {
            unget_token(&token);
            break;
        }
        v2 = parse_term();
        if (token.kind == ADD_OPERATOR_TOKEN)
        {
            v1 += v2;
        }
        else if (token.kind == SUB_OPERATOR_TOKEN)
        {
            v1 -= v2;
        }
    }
    return v1;
}

double
parse_line()
{
    double value;
    st_look_ahead_token_exists = 0;
    value = parse_expression();
    return value;
}

int main()
{
    char line[LINE_BUF_SIZE];
    double value;

    while (fgets(line, LINE_BUF_SIZE, stdin) != NULL)
    {
        set_line(line);
        value = parse_line();
        printf(">>%f\n", value);
    }
    return 0;
}
