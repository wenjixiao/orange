enum token_type {INTEGER2,INTEGER8,INTEGER10,INTEGER16,IDENTIFIER,BOOLEAN,STRING,LP,RP};

typedef struct token {
    enum token_type type;
    char* text;
    struct token *next;
}Token;

void print_tokens();
Token *get_tokens(FILE *f);
void destroy_tokens(Token* head);
