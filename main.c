#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>

#define STC_IMPLEMENTATION
#define STC_STRIP_PREFIX
#include <stc.h> 

typedef enum {
    TK_Error = 0,

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
    char *value; 
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
    StringBuilder n_char = {0};

    while(isdigit(lex->current_ch)) {
        n = n * 10 + lex->current_ch - '0';
        da_push(&n_char, lex->current_ch);

        consume_char(lex);
    }

    da_push(&lex->tokens, ((Token){
        .kind = TK_Number,
        .value = n_char.items,
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
        .value = sb.items,
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
            t.value = "+";
            break;
        case '-':
            t.kind = TK_Minus;
            t.value = "-";
            break;
        case '*':
            t.kind = TK_Ast;
            t.value = "*";
            break;
        case '/':
            t.kind = TK_Slash;
            t.value = "/";
            break;
        case ':':
            t.kind = TK_Colon;
            t.value = ":";
            break;
        case '=':
            t.kind = TK_Equals;
            t.value = "=";
            break;
        case '(':
            t.kind = TK_LeftParen;
            t.value = "(";
            break;
        case ')':
            t.kind = TK_RightParen;
            t.value = ")";
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

bool is_op(Token t) {
    bool is = false;
    switch(t.kind) {
        case TK_Plus:
        case TK_Minus:
        case TK_Ast:
        case TK_Slash:
            is = true;
            break;
        default:
            break;
    }
    return is;
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

typedef struct Expression Expression;

typedef enum {
    EK_NumberExpression,
    EK_BinaryExpression,
} ExpressionKind;

typedef unsigned long long NumberExpression;

typedef enum {
    Op_Plus,
    Op_Minus,
    Op_Ast,
    Op_Slash,
} Operator;

typedef struct {
    Expression *lhs;
    Operator op;
    Expression *rhs;
} BinaryExpression;

struct Expression {
    ExpressionKind kind;
    union {
        NumberExpression num;
        BinaryExpression bin;
    } as;
};

typedef enum {
    SK_Expression,
} StatementKind;

typedef struct {
    StatementKind kind;
    union {
        Expression expr;
    } as;
} Statement;

typedef struct {
    Statement *items;
    size_t count;
    size_t cap;
} Ast;

void print_ast(Ast ast) {
    println("- Ast:");
    for (size_t i = 0; i < ast.count; ++i) {
        Statement s = ast.items[i];
        switch (s.kind) {
            case SK_Expression:
                println("  - Expression:");
                Expression expr = s.as.expr;
                switch (expr.kind) {
                    case EK_NumberExpression:
                        println("    - NumberExpression: %ld", expr.as.num);
                        break;
                    case EK_BinaryExpression:
                        println("    - BinaryExpression");
                        println("      - lhs: %d", expr.as.bin.lhs->as.num);
                        printf("       - op: ");
                        switch (expr.as.bin.op) {
                            case Op_Plus:
                                println("Plus");
                                break;
                            case Op_Minus:
                                println("Minus");
                                break;
                            case Op_Ast:
                                println("Asterisk");
                                break;
                            case Op_Slash:
                                println("Slash");
                                break;
                        }
                        println("      - lhs: %d", expr.as.bin.rhs->as.num);
                        break;
                    default:
                        todo("Printing ast for that Expression is not implemented yet! Sry");
                }
                break;
            default:
                todo("Printing ast for that statement is not implemented yet! Sry");
        }

    }
}

typedef struct {
    Ast ast;
    Tokens *tokens;
    size_t pos;
} Parser;

Token consume_token(Parser *parser) {
    return parser->tokens->items[parser->pos++];
}

Expression *parse_expr(Parser *parser, bool look_ahead);
Statement *parse_stmt(Parser *parser);

NumberExpression *parse_number(Parser *parser) {
    Token t = consume_token(parser);
    NumberExpression *e = malloc(sizeof(NumberExpression));

    switch (t.kind) {
        case TK_Number:
            e = &t.as.num;
            break;
        default:
            log(STC_ERROR, "Expected NumberExpression");
            exit(1);
            break;
    }

    return e;
}

Operator parse_operator(Parser *parser) {
    Token t = consume_token(parser);
    switch (t.kind) {
        case TK_Plus:
            return Op_Plus;
            break;
        case TK_Minus:
            return Op_Minus;
            break;
        case TK_Ast:
            return Op_Ast;
            break;
        case TK_Slash:
            return Op_Slash;
            break;
        default:
            log(STC_ERROR, "Expected Operator");
            exit(1);
    }
}

BinaryExpression* parse_bin_expr(Parser *parser) {
    BinaryExpression *e = malloc(sizeof(BinaryExpression));
    e->lhs = parse_expr(parser, false);
    e->op = parse_operator(parser);
    e->rhs = parse_expr(parser, false);
    return e;
}

Expression *parse_expr(Parser *parser, bool look_ahead) {
    Token t = parser->tokens->items[parser->pos];
    Expression *e = malloc(sizeof(Expression));
    switch (t.kind) {
        case TK_Number:
            if (look_ahead && is_op(parser->tokens->items[parser->pos + 1])) {
                e->kind = EK_BinaryExpression;
                e->as.bin = *parse_bin_expr(parser);
            } else {
                e->kind = EK_NumberExpression;
                e->as.num = *parse_number(parser);
                break;
            }
            break;
        default:
            log(STC_ERROR, "`%s` is not an expression", t.value);
            exit(1);
    }
    return e;
}

Statement parse_statement(Parser *parser) {
    return (Statement) {
        .kind = SK_Expression,
        .as.expr = *parse_expr(parser, true),
    };
}

void parse(Parser *parser) {
    while (parser->pos < parser->tokens->count) {
        da_push(&parser->ast, parse_statement(parser));
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
    // TODO: Add a flag to output tokens and ast
    /*for (size_t i = 0; i < lexer.tokens.count; ++i) {*/
    /*    Token t = lexer.tokens.items[i];*/
    /*    print_token(t);*/
    /*}*/
    Parser parser = {
        .tokens = &lexer.tokens,
    };
    parse(&parser);
    print_ast(parser.ast);
    return 0;
}
