#include "interpreter.h"



static char peek(Scanner *scanner)
{
    return *scanner->current;
}

static char advance_scanner(Scanner *scanner)
{
    scanner->current++;
    return scanner->current[-1];
}

static bool isDigit(char c)
{
    return c >= '0' && c <= '9';
}

static bool isAlpha(char c) {
  return (c >= 'a' && c <= 'z') ||
         (c >= 'A' && c <= 'Z') ||
          c == '_';
}

static void skipWhitespace(Scanner *scanner)
{
    while (true)
    {
        char c = peek(scanner);
        switch (c)
        {
            case ' ':
            case '\r':
            case '\t':
                advance_scanner(scanner);
                break;
            case '\n':
                advance_scanner(scanner);
                break;
            default:
                return;
        }
    }
}


static void initScanner(Scanner *scanner, const char *source)
{
    scanner->start = source;
    scanner->current = source;
}

static bool is_at_end(Scanner *scanner)
{
    return *scanner->current == '\0';
}

static Token makeToken(Scanner *scanner, TokenType type)
{
    Token token;
    token.type = type;
    token.start = scanner->start;
    token.length = (int)(scanner->current - scanner->start);
    return token;
}

static TokenType checkKeyword(Scanner *scanner, int start, int length, const char *rest, TokenType type)
{
    if (scanner->current - scanner->start == start + length && memcmp(scanner->start + start, rest, length) == 0)
    {
        return type;
    }

    return TOKEN_PORT_PIN;
}

static TokenType identiferType(Scanner *scanner)
{
    switch (scanner->start[0])
    {
        case 'i':
        {
            return checkKeyword(scanner, 1, 4, "nput", TOKEN_GPIO_INPUT);
        }
        case 'o':
        {
            return checkKeyword(scanner, 1, 5, "utput", TOKEN_GPIO_OUTPUT);
        }
        case 's':
        {
            return checkKeyword(scanner, 1, 2, "et", TOKEN_GPIO_SET);
        }
        case 'r':
        {
            return checkKeyword(scanner, 1, 4, "eset", TOKEN_GPIO_RESET);
        }
        case 't':
        {
            return checkKeyword(scanner, 1, 5, "oggle", TOKEN_GPIO_TOGGLE);
        }
    }

    return TOKEN_PORT_PIN;
}

static Token identifier(Scanner *scanner)
{
    while (isAlpha(peek(scanner)) || isDigit(peek(scanner))) advance_scanner(scanner);
    return makeToken(scanner, identiferType(scanner));
}

static char *get_token_type_name(TokenType token)
{
    switch (token)
    {
        case TOKEN_EOL:
        {
            return "TOKEN_EOL";
        }
        case TOKEN_ERROR:
        {
            return "TOKEN_ERROR";
        }
        case TOKEN_GPIO_INPUT:
        {
            return "TOKEN_GPIO_INPUT";
        }
        case TOKEN_GPIO_OUTPUT:
        {
            return "TOKEN_GPIO_OUTPUT";
        }
        case TOKEN_GPIO_RESET:
        {
            return "TOKEN_GPIO_RESET";
        }
        case TOKEN_GPIO_SET:
        {
            return "TOKEN_GPIO_SET";
        }
        case TOKEN_GPIO_TOGGLE:
        {
            return "TOKEN_GPIO_TOGGLE";
        }
        case TOKEN_PORT_PIN:
        {
            return "TOKEN_PORT_PIN";
        }
    }
    return "UNKNOWN_TOKEN";
}

static void print_token(Token token)
{
    printf("Token type: %s\r\nToken: %.*s\r\n", get_token_type_name(token.type), token.length, token.start);
}

bool interpret(BoardController *bc, char *source, size_t length)
{
    Scanner scanner;
    initScanner(&scanner, source);
    int count = 0;
    Token tokvec[20];

    while (true)
    {
        skipWhitespace(&scanner);
        scanner.start = scanner.current;
        if (is_at_end(&scanner))
        {
            tokvec[count++] = makeToken(&scanner, TOKEN_EOL);
            printf("Token count = %d\r\n", count);
            for (int i = 0; i < count; i++)
            {
                print_token(tokvec[i]);
            }
            count = 0;

            return true;
        }

        char c = advance_scanner(&scanner);
        if (isAlpha(c)) tokvec[count++] = identifier(&scanner);
        else tokvec[count++] = makeToken(&scanner, TOKEN_ERROR);
    }
}

