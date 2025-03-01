#include <stdio.h>
#include <ctype.h>

#define STC_IMPLEMENTATION
#define STC_STRIP_PREFIX
#include <stc.h> 

typedef enum {
    TK_Error,

    TK_Number,
    TK_Ident,

    TK_Plus,
    TK_Minus,
    TK_Ast,
    TK_Slash,
    TK_Colon,
    TK_Equals,
    TK_LeftParen,
    TK_RightParen,
} TokenKind;

typedef struct {
    TokenKind kind;
    union {
        unsigned long long num;
        char *ident;
    } as;
    size_t row;
    size_t col;
} Token;

typedef struct {
    Token *items;
    size_t count;
    size_t cap;
} Tokens;

typedef struct {
    const char *input;
    Tokens tokens;
    char current_ch;
    size_t pos;
} Lexer;

char consume_char(Lexer *lex) {
    lex->current_ch = lex->input[lex->pos++];
    return lex->current_ch; 
}

void lex_number(Lexer *lex) {
    unsigned long long n = 0;

    while(isdigit(lex->current_ch)) {
        n = n * 10 + lex->current_ch - '0';

        consume_char(lex);
    }


    da_push(&lex->tokens, ((Token){
        .kind = TK_Number,
        .as = {
            .num = n,
        },
    }));
}

void lex_ident(Lexer *lex) {
    StringBuilder sb = {0};
    while (isalnum(lex->current_ch)) {
        da_push(&sb, lex->current_ch);
        consume_char(lex);
    }
    da_push(&sb, '\0');

    da_push(&lex->tokens, ((Token){
        .kind = TK_Ident,
        .as = {
            .ident = sb.items,
        },
    }));
}

void lex_punct(Lexer *lex) {
    char c = lex->current_ch;
    Token t;
    t.kind = TK_Error;
    switch (c) {
        case '+':
            t.kind = TK_Plus;
            break;
        case '-':
            t.kind = TK_Minus;
            break;
        case '*':
            t.kind = TK_Ast;
            break;
        case '/':
            t.kind = TK_Slash;
            break;
        case ':':
            t.kind = TK_Colon;
            break;
        case '=':
            t.kind = TK_Equals;
            break;
        case '(':
            t.kind = TK_LeftParen;
            break;
        case ')':
            t.kind = TK_RightParen;
            break;
        default:
            break;
    }
    consume_char(lex);
    da_push(&lex->tokens, t);
}

void lex(Lexer *lex) {
    if (!lex->input) return;
    consume_char(lex);
    while (lex->current_ch) {
        if (isspace(lex->current_ch)) {
            consume_char(lex);
            continue;
        }

        if (ispunct(lex->current_ch)) {
            lex_punct(lex);
            continue;
        }

        if (isdigit(lex->current_ch)) {
            lex_number(lex);
            continue;
        }

        if (isalnum(lex->current_ch)) {
            lex_ident(lex);
            continue;
        }

        da_push(&lex->tokens,(Token){.kind = TK_Error});
        consume_char(lex);
    }
}

void print_token(Token t) {
    switch (t.kind) {
        case TK_Number:
            log(STC_DEBUG, "Number: %ld", t.as.num);
            break;
        case TK_Ident:
            log(STC_DEBUG, "Ident: %s", t.as.ident);
            break;
        case TK_Plus:
            log(STC_DEBUG, "Plus");
            break;
        case TK_Minus:
            log(STC_DEBUG, "Minus");
            break;
        case TK_Ast:
            log(STC_DEBUG, "Asterisk");
            break;
        case TK_Slash:
            log(STC_DEBUG, "Slash");
            break;
        case TK_Error:
            log(STC_DEBUG, "Error");
            break;
        case TK_Colon:
            log(STC_DEBUG, "Colon");
            break;
        case TK_Equals:
            log(STC_DEBUG, "Equals");
            break;
        case TK_LeftParen:
            log(STC_DEBUG, "LeftParen");
            break;
        case TK_RightParen:
            log(STC_DEBUG, "RightParen");
            break;
        default:
            break;
    }
}

typedef enum {
    EK_NumberExpression,
} ExpressionKind;

typedef unsigned long long NumberExpression;

typedef struct {
    ExpressionKind kind;
    union {
        NumberExpression num;
    } as;
} Expression;

void print_expr(Expression expr) {
    switch (expr.kind) {
        case EK_NumberExpression:
            log(STC_DEBUG, "Expr: Number(%ld)", expr.as.num);
            break;
        default:
            todo(__LINE__);
    }
}

typedef enum {
    SK_Expression,
} StatementKind;

typedef struct {
    StatementKind kind;
    union {
        Expression expr;
    } as;
} Statement;

void print_stmt(Statement s) {
    switch (s.kind) {
        case SK_Expression:
            print_expr(s.as.expr);
            break;
        default:
            todo(__LINE__);
    }
}

typedef struct {
    Statement *items;
    size_t count;
    size_t cap;
} Statements;

typedef struct {
    Tokens tokens;
    Statements stmts;
    size_t pos;
} Ast;

Token consume_token(Ast *par) {
    return par->tokens.items[par->pos++];
}

void parse_number(Ast *par) {
    Token t = consume_token(par);

    switch (t.kind) {
        case TK_Number:
            da_push(&par->stmts, ((Statement){
                .kind = SK_Expression,
                .as.expr = (Expression) {
                    .kind = EK_NumberExpression,
                    .as.num = t.as.num,
                },
            }));
            break;
        default:
            log(STC_ERROR, "Expected NumberExpression");
            exit(1);
            break;
    }
}

void parse(Ast *par) {
    while (par->pos < par->tokens.count) {
        Token t = par->tokens.items[par->pos];
        switch (t.kind) {
            case TK_Number:
                parse_number(par);
                break;
            default:
                todo("Not implemented yet");
        }
    }
}
int main(int argc, char **argv) {
    (void*)shift(argv, argc);
    if (argc < 1) {
        log(STC_ERROR, "No input file provided");
        return 1;
    }
    char *input_filename = shift(argv, argc);
    StringBuilder file_content = {0};
    read_file(input_filename, &file_content);
    log(STC_DEBUG, "Read %d bytes from `%s`", file_content.count, input_filename);
    Lexer lexer = {
        .input = file_content.items,
    };
    lex(&lexer);
    for (size_t i = 0; i < lexer.tokens.count; ++i) {
        Token t = lexer.tokens.items[i];
        print_token(t);
    }
    Ast ast = {
        .tokens = lexer.tokens,
    };
    parse(&ast);
    for (size_t i = 0; i < ast.stmts.count; ++i) {
        Statement s = ast.stmts.items[i];
        print_stmt(s);
    }
    return 0;
}
