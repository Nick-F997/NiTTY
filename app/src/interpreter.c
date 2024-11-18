#include "interpreter.h"

/**
 * @brief Looks at the current character in a string.
 * 
 * @param scanner scanner to check next char
 * @return char 
 */
static char peek(Scanner *scanner)
{
    return *scanner->current;
}

/**
 * @brief increments current character to be analysed and returns previous character.
 * 
 * @param scanner scanner object to be advanced.
 * @return char 
 */
static char advance_scanner(Scanner *scanner)
{
    scanner->current++;
    return scanner->current[-1];
}

/**
 * @brief checks whether the character is a numerical symbol
 * 
 * @param c character to check
 * @return true c is digit
 * @return false c is not digit
 */
static bool isDigit(char c)
{
    return c >= '0' && c <= '9';
}

/**
 * @brief checks whether character is alphabetic.
 * 
 * @param c character to check
 * @return true character is alphabetic
 * @return false character is not alphabetic
 */
static bool isAlpha(char c) {
  return (c >= 'a' && c <= 'z') ||
         (c >= 'A' && c <= 'Z') ||
          c == '_';
}

/**
 * @brief Jumps over whitespace. 
 * 
 * @param scanner scanner to jump
 */
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

/**
 * @brief Initialises Scanner object
 * 
 * @param scanner object to be initialised
 * @param source source code to initialise to.
 */
static void initScanner(Scanner *scanner, const char *source)
{
    scanner->start = source;
    scanner->current = source;
}

/**
 * @brief Checks whether scanner is at the end of the line or not.
 * 
 * @param scanner scanner to check
 * @return true scanner is at end
 * @return false scanner is not at end
 */
static bool is_at_end(Scanner *scanner)
{
    return *scanner->current == '\0';
}

/**
 * @brief Creates token from current string segment
 * 
 * @param scanner scanner to create token from
 * @param type type of token
 * @return Token the created token.
 */
static Token makeToken(Scanner *scanner, TokenType type)
{
    Token token;
    token.type = type;
    token.start = scanner->start;
    token.length = (int)(scanner->current - scanner->start);
    return token;
}

/**
 * @brief Checks whether the current token in scanner matches the provided "rest of string". For example, 
 * if Scanner's current string starts with "i", and rest is given as "nput", if the rest of Scanner's string 
 * is "input", this function returns the "input" token. If it does not match, it returns the "identifier" token. 
 * 
 * @param scanner the scanner to check.
 * @param start From which character we are starting from (0 indexed)
 * @param length the length of the remaining string.
 * @param rest the rest of the string
 * @param type the type to be checked against.
 * @return TokenType 
 */
static TokenType checkKeyword(Scanner *scanner, int start, int length, const char *rest, TokenType type)
{
    // Ensure that the provided "rest of string" is both the same length and matches the text.
    if (scanner->current - scanner->start == start + length 
            && memcmp(scanner->start + start, rest, length) == 0)
    {
        return type;
    }

    return TOKEN_PORT_PIN;
}

/**
 * @brief Checks what the current string starts with and matches that to a token.
 * 
 * @param scanner scanner to be checked
 * @return TokenType the matched token
 */
static TokenType identiferType(Scanner *scanner)
{
    // Switch on first character in string.
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

/**
 * @brief Advances through current string until it encounters something that isn't a token.
 * Then analyses that string and returns the matching token.
 * 
 * @param scanner scanner to be checked
 * @return Token created token.
 */
static Token identifier(Scanner *scanner)
{
    while (isAlpha(peek(scanner)) || isDigit(peek(scanner))) advance_scanner(scanner);
    return makeToken(scanner, identiferType(scanner));
}

/**
 * @brief Function for debugging. Returns name of token.
 * 
 * @param token token to get name of
 * @return char* 
 */
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

/**
 * @brief Debugging function for printing token info
 * 
 * @param token token to be printed.
 */
static void print_token(Token token)
{
    printf("Token type: %s\r\nToken: %.*s\r\n", get_token_type_name(token.type), token.length, token.start);
}

/**
 * @brief Interprets a given line.
 * 
 * @param bc the board controller struct. 
 * @param source the line to be interpetered.
 * @param length length of the line, unused.
 * @return true successful interpretation
 * @return false unsucessful interpretation.
 */
bool interpret(BoardController *bc, char *source, size_t length)
{
    // initialise scanner obj
    Scanner scanner;
    initScanner(&scanner, source);
    // For debugging. Will replace with an actual vector at some point.
    int count = 0; // size of tokvec
    Token tokvec[20]; // Collection of tokens

    while (true)
    {
        skipWhitespace(&scanner); // Skip over whitespace
        scanner.start = scanner.current; // assign first character after whitespace to be the starting char.
        if (is_at_end(&scanner)) // When finished...
        {
            // Debugging stuff.
            tokvec[count++] = makeToken(&scanner, TOKEN_EOL);
            printf("Token count = %d\r\n", count);
            for (int i = 0; i < count; i++)
            {
                print_token(tokvec[i]);
            }
            count = 0;

            // Successful interpretation.
            return true;
        }

        // Move to the next character.
        char c = advance_scanner(&scanner);
        if (isAlpha(c)) tokvec[count++] = identifier(&scanner); // If its a letter start checking
        else tokvec[count++] = makeToken(&scanner, TOKEN_ERROR); // Otherwise stop
    }
}

