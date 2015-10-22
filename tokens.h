/*

(c) 1993 Roger Binns

These tools were produced by Roger Binns for a fourth year project as part of
a computer science degree, for the Computer Science department, Brunel
University, Uxbridge, Middlesex UB8 3PH, United Kingdom.

This software is provided in good faith, having been developed by Brunel
University students as part of their normal course work.  It should not be
assumed that Brunel has any rights of ownership, and the University cannot
accept any liability for its subsequent use.  It is a condition of any such
use that the user idemnifies the University against any claim (including
third party claims) arising therefrom.

*/

enum TOKENS
{
  NEWLINE = 256,
  ENDOFSTATEMENT,
  IF,
  WHILE,
  BEGINSCOPE,
  ENDSCOPE,
  DO,
  SWITCH,
  CASE,
  DEFAULT,
  GOTO,
  FUNCTION,
  LABEL,
  STRUCT,
  UNION,
  CLASS,
  FOR,
  BREAK,
  AND,
  OR,
  CONTINUE,
  RETURN,
  IGNORE,
  ELSE,
	ELSEIF
};

extern union YY_LVAL
{
  char *yy_str;
} yylval;

extern int atelparen;
