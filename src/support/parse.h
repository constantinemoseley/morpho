/** @file parse.h
 *  @author T J Atherton and others (see below)
 *
 *  @brief Parser
*/

#ifndef parse_h
#define parse_h

#include <stdio.h>
#include "error.h"
#include "lex.h"
#include "syntaxtree.h"

/* -------------------------------------------------------
 * Parser data types
 * ------------------------------------------------------- */

/** Parser type defined below */
typedef struct sparser parser;

/* -------------------------------------------------------
 * The parser is defined by parserules that respond to
 * various token types
 * ------------------------------------------------------- */

/** @brief an enumerated type that defines precedence order. */
enum {
    PREC_NONE,
    PREC_LOWEST,
    PREC_ASSIGN,
    PREC_OR,
    PREC_AND,
    PREC_EQUALITY,
    PREC_COMPARISON,
    PREC_RANGE,
    PREC_TERM,
    PREC_FACTOR,
    PREC_UNARY,
    PREC_POW,
    PREC_CALL,
    PREC_HIGHEST
};

/** Precedence order */
typedef int precedence;

/** @brief Definition of a parse function. */
typedef bool (*parsefunction) (parser *c, void *out);

/** @brief A parse rule will be defined for each token,
 * providing functions to parse the token if it is encountered in the
 * prefix or infix positions. The parse rule also defines the precedence. */
typedef struct {
    tokentype type;
    parsefunction prefix;
    parsefunction infix;
    precedence precedence;
} parserule;

/** @brief Macros used to build a parser definition table
 *  Each line in the table defines the parserule(s) for a specific token type.  */
#define PARSERULE_UNUSED(tok)                         { tok, NULL,    NULL,    PREC_NONE }
#define PARSERULE_PREFIX(tok, fn)                     { tok, fn,      NULL,    PREC_NONE }
#define PARSERULE_INFIX(tok, fn, prec)                { tok, NULL,    fn,      prec      }
#define PARSERULE_MIXFIX(tok, unaryfn, infixfn, prec) { tok, unaryfn, infixfn, prec      }

/** Varrays of parse rules */
DECLARE_VARRAY(parserule, parserule)

/* -------------------------------------------------------
 * Define a Parser
 * ------------------------------------------------------- */

/** @brief A structure that defines the state of a parser */
struct sparser {
    token current; /** The current token */
    token previous; /** The previous token */
    syntaxtreeindx left;
    lexer *lex; /** Lexer to use */
    void *out; /** Output */
    error *err; /** Error structure to output errors to */
    bool nl; /** Was a newline encountered before the current token? */
    varray_parserule parsetable;
};

/* -------------------------------------------------------
 * Parser error messages
 * ------------------------------------------------------- */

#define PARSE_INCOMPLETEEXPRESSION      "IncExp"
#define PARSE_INCOMPLETEEXPRESSION_MSG  "Incomplete expression."

#define PARSE_MISSINGPARENTHESIS        "MssngParen"
#define PARSE_MISSINGPARENTHESIS_MSG    "Expect ')' after expression."

#define PARSE_EXPECTEXPRESSION          "ExpExpr"
#define PARSE_EXPECTEXPRESSION_MSG      "Expected expression."

#define PARSE_MISSINGSEMICOLON          "MssngSemiVal"
#define PARSE_MISSINGSEMICOLON_MSG      "Expect ; after value."

#define PARSE_MISSINGSEMICOLONEXP       "MssngExpTerm"
#define PARSE_MISSINGSEMICOLONEXP_MSG   "Expect expression terminator (; or newline) after expression."

#define PARSE_MISSINGSEMICOLONVAR       "MssngSemiVar"
#define PARSE_MISSINGSEMICOLONVAR_MSG   "Expect ; after variable declaration."

#define PARSE_VAREXPECTED               "VarExpct"
#define PARSE_VAREXPECTED_MSG           "Variable name expected after var."

#define PARSE_BLOCKTERMINATOREXP        "MssngBrc"
#define PARSE_BLOCKTERMINATOREXP_MSG    "Expected '}' to finish block."

#define PARSE_IFLFTPARENMISSING         "IfMssngLftPrn"
#define PARSE_IFLFTPARENMISSING_MSG     "Expected '(' after if."

#define PARSE_IFRGHTPARENMISSING        "IfMssngRgtPrn"
#define PARSE_IFRGHTPARENMISSING_MSG    "Expected ')' after condition."

#define PARSE_WHILELFTPARENMISSING      "WhlMssngLftPrn"
#define PARSE_WHILELFTPARENMISSING_MSG  "Expected '(' after while."

#define PARSE_FORLFTPARENMISSING        "ForMssngLftPrn"
#define PARSE_FORLFTPARENMISSING_MSG    "Expected '(' after for."

#define PARSE_FORSEMICOLONMISSING       "ForMssngSemi"
#define PARSE_FORSEMICOLONMISSING_MSG   "Expected ';'."

#define PARSE_FORRGHTPARENMISSING       "ForMssngRgtPrn"
#define PARSE_FORRGHTPARENMISSING_MSG   "Expected ')' after for clauses."

#define PARSE_FNNAMEMISSING             "FnNoName"
#define PARSE_FNNAMEMISSING_MSG         "Expected function or method name."

#define PARSE_FNLEFTPARENMISSING        "FnMssngLftPrn"
#define PARSE_FNLEFTPARENMISSING_MSG    "Expect '(' after name."

#define PARSE_FNRGHTPARENMISSING        "FnMssngRgtPrn"
#define PARSE_FNRGHTPARENMISSING_MSG    "Expect ')' after parameters."

#define PARSE_FNLEFTCURLYMISSING        "FnMssngLftBrc"
#define PARSE_FNLEFTCURLYMISSING_MSG    "Expect '{' before body."

#define PARSE_CALLRGHTPARENMISSING      "CllMssngRgtPrn"
#define PARSE_CALLRGHTPARENMISSING_MSG  "Expect ')' after arguments."

#define PARSE_EXPECTCLASSNAME           "ClsNmMssng"
#define PARSE_EXPECTCLASSNAME_MSG       "Expect class name."

#define PARSE_CLASSLEFTCURLYMISSING     "ClsMssngLftBrc"
#define PARSE_CLASSLEFTCURLYMISSING_MSG "Expect '{' before class body."

#define PARSE_CLASSRGHTCURLYMISSING     "ClsMssngRgtBrc"
#define PARSE_CLASSRGHTCURLYMISSING_MSG "Expect '}' after class body."

#define PARSE_EXPECTDOTAFTERSUPER       "ExpctDtSpr"
#define PARSE_EXPECTDOTAFTERSUPER_MSG   "Expect '.' after 'super'"

#define PARSE_INCOMPLETESTRINGINT       "IntrpIncmp"
#define PARSE_INCOMPLETESTRINGINT_MSG   "Incomplete string after interpolation."

#define PARSE_VARBLANKINDEX             "EmptyIndx"
#define PARSE_VARBLANKINDEX_MSG         "Empty capacity in variable declaration."

#define PARSE_IMPORTMISSINGNAME         "ImprtMssngNm"
#define PARSE_IMPORTMISSINGNAME_MSG     "Import expects a module or file name."

#define PARSE_IMPORTUNEXPCTDTOK         "ImprtExpctFrAs"
#define PARSE_IMPORTUNEXPCTDTOK_MSG     "Import expects a module or file name followed by for or as."

#define PARSE_IMPORTASSYMBL             "ExpctSymblAftrAs"
#define PARSE_IMPORTASSYMBL_MSG         "Expect symbol after as in import."

#define PARSE_IMPORTFORSYMBL            "ExpctSymblAftrFr"
#define PARSE_IMPORTFORSYMBL_MSG        "Expect symbol(s) after for in import."

#define PARSE_EXPECTSUPER               "SprNmMssng"
#define PARSE_EXPECTSUPER_MSG           "Expect superclass name."

#define PARSE_UNRECGNZEDTOK               "UnrcgnzdTok"
#define PARSE_UNRECGNZEDTOK_MSG           "Encountered an unrecognized token."

#define PARSE_DCTSPRTR                    "DctSprtr"
#define PARSE_DCTSPRTR_MSG                "Expected a colon separating a key/value pair in dictionary."

#define PARSE_SWTCHSPRTR                  "SwtchSprtr"
#define PARSE_SWTCHSPRTR_MSG              "Expected a colon after label."

#define PARSE_DCTENTRYSPRTR               "DctEntrySprtr"
#define PARSE_DCTENTRYSPRTR_MSG           "Expected a comma or '}'."

#define PARSE_EXPCTWHL                    "ExpctWhl"
#define PARSE_EXPCTWHL_MSG                "Expected while after loop body."

#define PARSE_EXPCTCTCH                   "ExpctCtch"
#define PARSE_EXPCTCTCH_MSG               "Expected catch after try statement."

#define PARSE_CATCHLEFTCURLYMISSING       "ExpctHndlr"
#define PARSE_CATCHLEFTCURLYMISSING_MSG   "Expected block of error handlers after catch."

#define PARSE_ONEVARPR                    "OneVarPr"
#define PARSE_ONEVARPR_MSG                "Functions can have only one variadic parameter."

/* -------------------------------------------------------
 * Interface for writing a custom parser
 * ------------------------------------------------------- */

// Library functions
void parse_error(parser *p, bool use_prev, errorid id, ... );
bool parse_advance(parser *p);
bool parse_checktoken(parser *p, tokentype type);
bool parse_checktokenmulti(parser *p, int n, tokentype *type);
bool parse_checktokenadvance(parser *p, tokentype type);
bool parse_checkrequiredtoken(parser *p, tokentype type, errorid id);

// Find a parserule for a given tokentype
parserule *parse_getrule(parser *p, tokentype type);

/* -------------------------------------------------------
 * Parser interface
 * ------------------------------------------------------- */

void parse_init(parser *p, lexer *lex, error *err, void *out);
void parse_clear(parser *p);

bool parse(parser *p);

bool parse_stringtovaluearray(char *string, unsigned int nmax, value *v, unsigned int *n, error *err);

void parse_initialize(void);
void parse_finalize(void);

#endif /* parse_h */
