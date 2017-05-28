#include "c.h"

enum
{
    BLANK = 01,
    NEWLINE = 02,
    LETTER = 04,
    DIGIT = 010,
    HEX = 020,
    OTHER = 040
};

Coordinate src; /* current source coordinate */
int t;
char *token; /* current token */
Symbol tsym; /* symbol table entry for current token */

static unsigned char map[256] = {
    /* 000 nul */ 0,
    /* 001 soh */ 0,
    /* 002 stx */ 0,
    /* 003 etx */ 0,
    /* 004 eot */ 0,
    /* 005 enq */ 0,
    /* 006 ack */ 0,
    /* 007 bel */ 0,
    /* 010 bs  */ 0,
    /* 011 ht  */ BLANK,
    /* 012 nl  */ NEWLINE,
    /* 013 vt  */ BLANK,
    /* 014 ff  */ BLANK,
    /* 015 cr  */ 0,
    /* 016 so  */ 0,
    /* 017 si  */ 0,
    /* 020 dle */ 0,
    /* 021 dc1 */ 0,
    /* 022 dc2 */ 0,
    /* 023 dc3 */ 0,
    /* 024 dc4 */ 0,
    /* 025 nak */ 0,
    /* 026 syn */ 0,
    /* 027 etb */ 0,
    /* 030 can */ 0,
    /* 031 em  */ 0,
    /* 032 sub */ 0,
    /* 033 esc */ 0,
    /* 034 fs  */ 0,
    /* 035 gs  */ 0,
    /* 036 rs  */ 0,
    /* 037 us  */ 0,
    /* 040 sp  */ BLANK,
    /* 041 !   */ OTHER,
    /* 042 "   */ OTHER,
    /* 043 #   */ OTHER,
    /* 044 $   */ 0,
    /* 045 %   */ OTHER,
    /* 046 &   */ OTHER,
    /* 047 '   */ OTHER,
    /* 050 (   */ OTHER,
    /* 051 )   */ OTHER,
    /* 052 *   */ OTHER,
    /* 053 +   */ OTHER,
    /* 054 ,   */ OTHER,
    /* 055 -   */ OTHER,
    /* 056 .   */ OTHER,
    /* 057 /   */ OTHER,
    /* 060 0   */ DIGIT,
    /* 061 1   */ DIGIT,
    /* 062 2   */ DIGIT,
    /* 063 3   */ DIGIT,
    /* 064 4   */ DIGIT,
    /* 065 5   */ DIGIT,
    /* 066 6   */ DIGIT,
    /* 067 7   */ DIGIT,
    /* 070 8   */ DIGIT,
    /* 071 9   */ DIGIT,
    /* 072 :   */ OTHER,
    /* 073 ;   */ OTHER,
    /* 074 <   */ OTHER,
    /* 075 =   */ OTHER,
    /* 076 >   */ OTHER,
    /* 077 ?   */ OTHER,
    /* 100 @   */ 0,
    /* 101 A   */ LETTER | HEX,
    /* 102 B   */ LETTER | HEX,
    /* 103 C   */ LETTER | HEX,
    /* 104 D   */ LETTER | HEX,
    /* 105 E   */ LETTER | HEX,
    /* 106 F   */ LETTER | HEX,
    /* 107 G   */ LETTER,
    /* 110 H   */ LETTER,
    /* 111 I   */ LETTER,
    /* 112 J   */ LETTER,
    /* 113 K   */ LETTER,
    /* 114 L   */ LETTER,
    /* 115 M   */ LETTER,
    /* 116 N   */ LETTER,
    /* 117 O   */ LETTER,
    /* 120 P   */ LETTER,
    /* 121 Q   */ LETTER,
    /* 122 R   */ LETTER,
    /* 123 S   */ LETTER,
    /* 124 T   */ LETTER,
    /* 125 U   */ LETTER,
    /* 126 V   */ LETTER,
    /* 127 W   */ LETTER,
    /* 130 X   */ LETTER,
    /* 131 Y   */ LETTER,
    /* 132 Z   */ LETTER,
    /* 133 [   */ OTHER,
    /* 134 \   */ OTHER,
    /* 135 ]   */ OTHER,
    /* 136 ^   */ OTHER,
    /* 137 _   */ LETTER,
    /* 140 `   */ 0,
    /* 141 a   */ LETTER | HEX,
    /* 142 b   */ LETTER | HEX,
    /* 143 c   */ LETTER | HEX,
    /* 144 d   */ LETTER | HEX,
    /* 145 e   */ LETTER | HEX,
    /* 146 f   */ LETTER | HEX,
    /* 147 g   */ LETTER,
    /* 150 h   */ LETTER,
    /* 151 i   */ LETTER,
    /* 152 j   */ LETTER,
    /* 153 k   */ LETTER,
    /* 154 l   */ LETTER,
    /* 155 m   */ LETTER,
    /* 156 n   */ LETTER,
    /* 157 o   */ LETTER,
    /* 160 p   */ LETTER,
    /* 161 q   */ LETTER,
    /* 162 r   */ LETTER,
    /* 163 s   */ LETTER,
    /* 164 t   */ LETTER,
    /* 165 u   */ LETTER,
    /* 166 v   */ LETTER,
    /* 167 w   */ LETTER,
    /* 170 x   */ LETTER,
    /* 171 y   */ LETTER,
    /* 172 z   */ LETTER,
    /* 173 {   */ OTHER,
    /* 174 |   */ OTHER,
    /* 175 }   */ OTHER,
    /* 176 ~   */ OTHER,
};

int gettok(void)
{
    for (;;)
    {
        register unsigned char *rcp = cp;
        /* skip white space */
        while (map[*rcp] & blank)
            rcp++;
        if (limit - rcp < MAXTOKEN)
        {
            /* in case token may be splitted */
            cp = rcp;
            fillbuf();
            rcp = cp;
        }
        src.file = file;
        src.x = (char *)rcp - line;
        src.y = lineno;
        cp = rcp + 1;
        switch (*rcp++)
        {
        case '/':
            /* comment or / */
            if (*rcp == '*')
            {

                int c = 0;
                for (rcp++; *rcp != '/' || c != '*';)
                {
                    /* exit when c == '*' and rcp == '/' */
                    if (map[*rcp] & NEWLINE)
                    {
                        if (rcp < limit)
                            c = *rcp;
                        cp = rcp + 1;
                        nextline();
                        rcp = cp;
                        if (rcp == limit)
                            break;
                    }
                    else
                        c = *rcp++;
                }
                if (rcp < limit)
                    rcp++;
                else
                    printf("unclosed comment\n");
                cp = rcp;
                continue;
            }
            return '/';
        case '<':
            if (*rcp == '=')
                return cp++, LEQ;
            if (*rcp == '<')
                return cp++, LSHIFT;
            return '<';
        case '>':
            if (*rcp == '=')
                return cp++, GEQ;
            if (*rcp == '>')
                return cp++, RSHIFT;
            return '>';
        case '-':
            if (*rcp == '>')
                return cp++, DEREF;
            if (*rcp == '-')
                return cp++, DECR;
            return '-';
        case '=':
            return *rcp == '=' ? cp++, EQL : '=';
        case '!':
            return *rcp == '=' ? cp++, NEQ : '!';
        case '|':
            return *rcp == '|' ? cp++, OROR : '|';
        case '&':
            return *rcp == '&' ? cp++, ANDAND : '&';
        case '+':
            return *rcp == '+' ? cp++, INCR : '+';
        case ';':
        case ',':
        case ':':
        case '*':
        case '~':
        case '%':
        case '^':
        case '?':
        case '[':
        case ']':
        case '{':
        case '}':
        case '(':
        case ')':
            return rcp[-1];
        case '\n':
        case '\v':
        case '\r':
        case '\f':
            nextline();
            if (cp == limit)
            {
                tsym = NULL;
                return EOI;
            }
            continue;
        case 'i':
            if (rcp[0] == 'f' && !(map[rcp[1]] & (DIGIT | LETTER)))
            {
                /* "if" */
                cp = rcp + 1;
                return IF;
            }
            if (rcp[0] == 'n' && rcp[1] == 't' && !(map[rcp[2]] & (DIGIT | LETTER)))
            {
                /* "int" */
                cp = rcp + 2;
                tsym = inttype->u.sym; //TODO
                return INT;
            }
            goto id;
        case 'h':
        case 'j':
        case 'k':
        case 'm':
        case 'n':
        case 'o':
        case 'p':
        case 'q':
        case 'x':
        case 'y':
        case 'z':
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
        case 'G':
        case 'H':
        case 'I':
        case 'J':
        case 'K':
        case 'M':
        case 'N':
        case 'O':
        case 'P':
        case 'Q':
        case 'R':
        case 'S':
        case 'T':
        case 'U':
        case 'V':
        case 'W':
        case 'X':
        case 'Y':
        case 'Z':
        id:
            if (limit - rcp < MAXLINE)
            {
                cp = rcp - 1;
                fillbuf();
                rcp = ++cp;
            }
            assert(cp == rcp);
            token = (char *)rcp - 1;
            while (map[*rcp] & (DIGIT | LETTER))
                rcp++;
            token = stringn(token, (char *)rcp - token);
            tsym = lookup(token, identifiers); //TODO
            cp = rcp;
            return ID;
        default:
            if ((map[cp[-1]] & BLANK) == 0)
                if (cp[-1] < ' ' || cp[-1] >= 0177)
                    printf("illegal character `\\0%o'\n", cp[-1]);
                else
                    printf("illegal character `%c'\n", cp[-1]);
        }
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    {
        unsigned long n = 0;
        if (limit - rcp < MAXLINE)
        {
            cp = rcp - 1;
            fillbuf();
            rcp = ++cp;
        }
        assert(cp == rcp);
        token = (char *)rcp - 1;
        if (*token == '0' && (*rcp == 'x' || *rcp == 'X'))
        {
            /* hex */
            int d, overflow = 0;
            while (*++rcp)
            {
                if (map[*rcp] & DIGIT)
                    d = *rcp - '0';
                else if (*rcp >= 'a' && *rcp <= 'f')
                    d = *rcp - 'a' + 10;
                else if (*rcp >= 'A' && *rcp <= 'F')
                    d = *rcp - 'A' + 10;
                else
                    break;
                if (n & ~(~0UL >> 4))
                    overflow = 1;
                else
                    n = (n << 4) + d;
            }
            if ((char *)rcp - token <= 2)
                error("invalid hexadecimal constant `%S'\n", token, (char *)rcp - token);
            cp = rcp;
            tsym = icon(n, overflow, 16);
        }
        else if (*token == '0')
        {
            /* oct */
            int err = 0, overflow = 0;
            for (; map[*rcp] & DIGIT; rcp++)
            {
                if (*rcp == '8' || *rcp == '9')
                    err = 1;
                if (n & ~(~0UL >> 3))
                    overflow = 1;
                else
                    n = (n << 3) + (*rcp - '0');
            }
            if (*rcp == '.' || *rcp == 'e' || *rcp == 'E')
            {
                cp = rcp;
                tsym = fcon();
                return FCON;
            }
            cp = rcp;
            tsym = icon(n, overflow, 8);
            if (err)
                error("invalid octal constant `%S'\n", token, (char *)cp - token);
        }
        else
        {
            int overflow = 0;
            for (n = *token - '0'; map[*rcp] & DIGIT;)
            {
                int d = *rcp++ - '0';
                if (n > (ULONG_MAX - d) / 10)
                    overflow = 1;
                else
                    n = 10 * n + d;
            }
            if (*rcp == '.' || *rcp == 'e' || *rcp == 'E')
            {
                cp = rcp;
                tsym = fcon();
                return FCON;
            }
            cp = rcp;
            tsym = icon(n, overflow, 10);
        }
        return ICON;
    }
    }
}