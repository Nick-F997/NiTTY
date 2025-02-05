/**
 * @file interpreter.c
 * @author Nicholas Fairburn (nicholas2.fairburn@live.uwe.ac.uk)
 * @brief Logic for command line interpretation. It's more of a scanner than a full interpreter.
 * @version 0.1
 * @date 2024-11-18
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "interpreter.h"
#include "token.h"

/**
 * @brief Looks at the current character in a string.
 *
 * @param scanner scanner to check next char
 * @return char
 */
static char peek(Scanner *scanner) { return *scanner->current; }

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
static bool isDigit(char c) { return c >= '0' && c <= '9'; }

/**
 * @brief checks whether character is alphabetic.
 *
 * @param c character to check
 * @return true character is alphabetic
 * @return false character is not alphabetic
 */
static bool isAlpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }

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
static bool is_at_end(Scanner *scanner) { return *scanner->current == '\0'; }

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
 * @brief Similar to isAlpha(), but only between the first and last letter that indicates a port.
 *        for example on the STM32F411RE the first port is A, the last port is C.
 *
 * @param c character to check
 * @return true character is valid port identifier
 * @return false character is not valid port identifier
 */
static bool isValidPortPinStartingChar(char c)
{
    return (c >= PORTa_STM32F411RE && c <= PORTe_STM32F411RE) ||
           (c >= PORTA_STM32F411RE && c <= PORTE_STM32F411RE);
}

/**
 * @brief Function to check whether an indeterminate string is a port-pin identifier (e.g., A10 or
 * E09) or garbage input.
 *
 * @param scanner scanner to check
 * @return TokenType valid or error token.
 */
static TokenType isValidPortPin(Scanner *scanner)
{
    if (isValidPortPinStartingChar(scanner->start[0]))
    {
        if (scanner->start[1] == PIN0)
        {

            if (scanner->start[2] >= PIN0 && scanner->start[2] <= PIN9)
            {
                return TOKEN_PORT_PIN;
            }
        }
        else if (scanner->start[1] == PIN10)
        {
            if (scanner->start[2] >= PIN0 && scanner->start[2] <= PIN15)
            {
                return TOKEN_PORT_PIN;
            }
        }
    }

    return TOKEN_ERROR;
}

/**
 * @brief Checks whether the current token in scanner matches the provided "rest of string". For
 * example, if Scanner's current string starts with "i", and rest is given as "nput", if the rest of
 * Scanner's string is "input", this function returns the "input" token. If it does not match, it
 * returns the "identifier" token.
 *
 * @param scanner the scanner to check.
 * @param start From which character we are starting from (0 indexed)
 * @param length the length of the remaining string.
 * @param rest the rest of the string
 * @param type the type to be checked against.
 * @return TokenType
 */
static TokenType checkKeyword(Scanner *scanner, int start, int length, const char *rest,
                              TokenType type)
{
    // Ensure that the provided "rest of string" is both the same length and matches the text.
    if (scanner->current - scanner->start == start + length &&
        memcmp(scanner->start + start, rest, length) == 0)
    {
        return type;
    }

    return isValidPortPin(scanner);
}

/**
 * @brief Checks what the current string starts with and matches that to a token.
 *
 * @param scanner scanner to be checked
 * @return TokenType the matched token
 */
static TokenType identifierType(Scanner *scanner)
{
    // Switch on first character in string.
    switch (scanner->start[0])
    {
    case 'a':
        return checkKeyword(scanner, 1, 2, "dc", TOKEN_ADC);
    case 'i':
        return checkKeyword(scanner, 1, 4, "nput", TOKEN_GPIO_INPUT);
    case 'n':
        return checkKeyword(scanner, 1, 3, "one", TOKEN_GPIO_NORESISTOR);
    case 'o':
        return checkKeyword(scanner, 1, 5, "utput", TOKEN_GPIO_OUTPUT);
    case 's':
        return checkKeyword(scanner, 1, 2, "et", TOKEN_GPIO_SET);
    case 'r':
    { // This is all a bit silly but it keeps in theme with the rest of the code.
        if (scanner->current - scanner->start > 1)
        {
            switch (scanner->start[1])
            {
            case 'e':
            {
                if (scanner->current - scanner->start > 2)
                {
                    switch (scanner->start[2])
                    {
                    case 'a':
                        return checkKeyword(scanner, 3, 1, "d", TOKEN_GPIO_READ);
                    case 's':
                        return checkKeyword(scanner, 3, 2, "et", TOKEN_GPIO_RESET);
                    }
                }
            }
            }
        }
        break;
    }
    case 'p':
    {
        if (scanner->current - scanner->start > 1)
        {
            switch (scanner->start[1])
            {
            case 'u':
                return checkKeyword(scanner, 2, 1, "p", TOKEN_GPIO_PULLUP);
            case 'd':
                return checkKeyword(scanner, 2, 3, "own", TOKEN_GPIO_PULLDOWN);
            }
        }
        break;
    }
    case 't':
        return checkKeyword(scanner, 1, 5, "oggle", TOKEN_GPIO_TOGGLE);
    case 'w':
        return checkKeyword(scanner, 1, 4, "rite", TOKEN_WRITE);
    }


    return isValidPortPin(scanner);
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
    while (isAlpha(peek(scanner)) || isDigit(peek(scanner)))
        advance_scanner(scanner);
    return makeToken(scanner, identifierType(scanner));
}

/**
 * @brief If it encounters a digit, the scanner attempts to parse it. Does not convert it from
 * string.
 *
 * @param scanner scanner to check
 * @return Token number token.
 */
static Token number(Scanner *scanner)
{
    while (isDigit(peek(scanner)))
        advance_scanner(scanner);
    return makeToken(scanner, TOKEN_NUMBER);
}

/**
 * @brief Creates a string from a token.
 *
 * @param scanner scanner to check
 * @return Token either string token or error if string is unterminated.
 */
static Token string(Scanner *scanner)
{
    while (peek(scanner) != '"' && !is_at_end(scanner))
    {
        advance_scanner(scanner);
    }
    if (is_at_end(scanner))
    {
        return makeToken(scanner, TOKEN_ERROR);
    }
    advance_scanner(scanner); // Consume second " token
    return makeToken(scanner, TOKEN_STRING);
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
    case TOKEN_ADC:
    {
        return "TOKEN_ADC";
    }
    case TOKEN_GPIO_INPUT:
    {
        return "TOKEN_GPIO_INPUT";
    }
    case TOKEN_GPIO_OUTPUT:
    {
        return "TOKEN_GPIO_OUTPUT";
    }
    case TOKEN_GPIO_READ:
    {
        return "TOKEN_GPIO_READ";
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
    case TOKEN_NUMBER:
    {
        return "TOKEN_NUMBER";
    }
    case TOKEN_STRING:
    {
        return "TOKEN_STRING";
    }
    case TOKEN_PORT_PIN:
    {
        return "TOKEN_PORT_PIN";
    }
    case TOKEN_GPIO_NORESISTOR:
    {
        return "TOKEN_GPIO_NORESISTOR";
    }
    case TOKEN_GPIO_PULLUP:
    {
        return "TOKEN_GPIO_PULLUP";
    }
    case TOKEN_GPIO_PULLDOWN:
    {
        return "TOKEN_GPIO_PULLDOWN";
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
    if (token.type == TOKEN_ERROR)
    {
        printf("> Scanner Error: Could not parse \"%.*s\". Unknown keyword or GPIO port-pin "
               "identifier.\r\n",
               token.length, token.start);
    }
    else
    {
        printf("TOKEN TYPE: %s\r\nTOKEN: %.*s\r\n", get_token_type_name(token.type), token.length,
               token.start);
    }
}

/**
 * @brief Interprets a given line.
 *
 * @param bc the board controller struct.
 * @param source the line to be interpetered.
 * @param length length of the line, unused.
 * @return true successful interpretation
 * @return false unsuccessful interpretation.
 */
bool interpret(BoardController *bc, char *source, size_t /*unused*/ length)
{
    // initialise scanner obj
    Scanner scanner;
    initScanner(&scanner, source);

    // Initialise token vector
    TokenVector *tokvec = initTokenVector();

    while (true)
    {
        skipWhitespace(&scanner); // Skip over whitespace
        scanner.start =
            scanner.current;     // assign first character after whitespace to be the starting char.
        if (is_at_end(&scanner)) // When finished...
        {
            // Debugging stuff.
            appendTokenVector(tokvec, makeToken(&scanner, TOKEN_EOL));
            break;
        }

        // Move to the next character.
        char  c = advance_scanner(&scanner);
        Token token;

        if (isAlpha(c)) // If its a letter start checking
        {
            token = identifier(&scanner);
        }
        else if (isDigit(c)) // If it's a number parse it as an int.
        {
            token = number(&scanner);
        }
        else if (c == '"') // If we encounter a string indicator create a string.
        {
            token = string(&scanner);
        }
        else
        {
            token = makeToken(&scanner, TOKEN_ERROR); // Otherwise stop
        }

        appendTokenVector(tokvec, token);

        if (token.type == TOKEN_ERROR)
        {
            print_token(token);
            deinitTokenVector(tokvec);
            return false;
        }
    }

    // After tokenisation completed
    // printf("Token count = %d\r\n", sizeTokenVector(tokvec));
    // for (size_t i = 0; i < sizeTokenVector(tokvec); i++)
    // {
    //     print_token(getTokenVector(tokvec, i));
    // }

    // Check interpretation and parsing.
    bool return_value = parseTokensAndExecute(bc, tokvec);
    deinitTokenVector(tokvec);
    return return_value;
}
