/*
	Aseba - an event-based framework for distributed robot control
	Copyright (C) 2007--2010:
		Stephane Magnenat <stephane at magnenat dot net>
		(http://stephane.magnenat.net)
		and other contributors, see authors.txt for details
	
	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU Lesser General Public License as published
	by the Free Software Foundation, version 3 of the License.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Lesser General Public License for more details.
	
	You should have received a copy of the GNU Lesser General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "compiler.h"
#include <cstdlib>
#include <sstream>
#include <ostream>
#include <cctype>
#include <cstdio>

namespace Aseba
{
	//! Construct a new token of given type and value
	Compiler::Token::Token(Type type, SourcePos pos, const std::string& value) :
		type(type),
		sValue(value),
		pos(pos)
	{
		if (type == TOKEN_INT_LITERAL)
		{
			bool wasSigned = false;
			if ((value.length() > 1) && (value[1] == 'x'))
				iValue = strtol(value.c_str() + 2, NULL, 16);
			else if ((value.length() > 1) && (value[1] == 'b'))
				iValue = strtol(value.c_str() + 2, NULL, 2);
			else
			{
				iValue = atoi(value.c_str());
				wasSigned = true;
			}
			if ((wasSigned == false) && (iValue > 32767))
				iValue -= 65536;
		}
		else
			iValue = 0;
		pos.column--; // column has already been incremented when token is created, so we remove one
		pos.character--; // character has already been incremented when token is created, so we remove one
	}
	
	//! Return the name of the type of this token
	const char* Compiler::Token::typeName() const
	{
		switch (type)
		{
			case TOKEN_END_OF_STREAM: return "end of stream";
			case TOKEN_STR_when: return "when keyword";
			case TOKEN_STR_emit: return "emit keyword";
			case TOKEN_STR_for: return "for keyword";
			case TOKEN_STR_in: return "in keyword";
			case TOKEN_STR_step: return "step keyword";
			case TOKEN_STR_while: return "while keyword";
			case TOKEN_STR_do: return "do keyword";
			case TOKEN_STR_if: return "if keyword";
			case TOKEN_STR_then: return "then keyword";
			case TOKEN_STR_else: return "else keyword";
			case TOKEN_STR_elseif: return "elseif keyword";
			case TOKEN_STR_end: return "end keyword";
			case TOKEN_STR_var: return "var keyword";
			case TOKEN_STR_call: return "call keyword";
			case TOKEN_STR_sub: return "sub keyword";
			case TOKEN_STR_callsub: return "callsub keyword";
			case TOKEN_STR_onevent: return "onevent keyword";
			case TOKEN_STR_abs: return "abs keyword";
			case TOKEN_STRING_LITERAL: return "string";
			case TOKEN_INT_LITERAL: return "integer";
			case TOKEN_PAR_OPEN: return "( (open parenthesis)";
			case TOKEN_PAR_CLOSE: return ") (close parenthesis)";
			case TOKEN_BRACKET_OPEN: return "[ (open bracket)";
			case TOKEN_BRACKET_CLOSE: return "] (close bracket)";
			case TOKEN_COLON: return ": (colon)";
			case TOKEN_COMMA: return ", (comma)";
			case TOKEN_ASSIGN: return "= (assignation)";
			case TOKEN_OP_OR: return "or";
			case TOKEN_OP_AND: return "and";
			case TOKEN_OP_NOT: return "not";
			case TOKEN_OP_BIT_OR: return "binary or";
			case TOKEN_OP_BIT_XOR: return "binary xor";
			case TOKEN_OP_BIT_AND: return "binary and";
			case TOKEN_OP_BIT_NOT: return "binary not";
			case TOKEN_OP_EQUAL: return "== (equal to)";
			case TOKEN_OP_NOT_EQUAL: return "!= (not equal to)";
			case TOKEN_OP_BIGGER: return "> (bigger than)";
			case TOKEN_OP_BIGGER_EQUAL: return ">= (bigger or equal than)";
			case TOKEN_OP_SMALLER: return "< (smaller than)";
			case TOKEN_OP_SMALLER_EQUAL: return "<= (smaller or equal than)";
			case TOKEN_OP_SHIFT_LEFT: return "<< (shift left)";
			case TOKEN_OP_SHIFT_RIGHT: return ">> (shift right)";
			case TOKEN_OP_ADD: return "+ (plus)";
			case TOKEN_OP_NEG: return "- (minus)";
			case TOKEN_OP_MULT: return "* (time)";
			case TOKEN_OP_DIV: return "/ (divide)";
			case TOKEN_OP_MOD: return "modulo";
			default: return "unknown";
		}
	}
	
	//! Return a string representation of the token
	std::string Compiler::Token::toString() const
	{
		std::ostringstream oss;
		oss << "Line: " << pos.row + 1 << " Col: " << pos.column + 1 << " : ";
		oss << typeName();
		if (type == TOKEN_INT_LITERAL)
			oss << " : " << iValue;
		if (type == TOKEN_STRING_LITERAL)
			oss << " : " << sValue;
		return oss.str();
	}
	
	
	//! Parse source and build tokens vector
	//! \param source source code
	void Compiler::tokenize(std::wistream& source)
	{
		tokens.clear();
		SourcePos pos(0, 0, 0);
		const unsigned tabSize = 4;
		
		// tokenize text source
		while (source.good())
		{
			wchar_t c = source.get();
			
			if (source.eof())
				break;
			
			pos.column++;
			pos.character++;
			
			switch (c)
			{
				// simple cases of one character
				case ' ': break;
				//case '\t': pos.column += tabSize - 1; break;
				case '\t': break;
				case '\n': pos.row++; pos.column = -1; break; // -1 so next call to pos.column++ result set 0
				case '\r': pos.column = -1; break; // -1 so next call to pos.column++ result set 0
				case '(': tokens.push_back(Token(Token::TOKEN_PAR_OPEN, pos)); break;
				case ')': tokens.push_back(Token(Token::TOKEN_PAR_CLOSE, pos)); break;
				case '[': tokens.push_back(Token(Token::TOKEN_BRACKET_OPEN, pos)); break;
				case ']': tokens.push_back(Token(Token::TOKEN_BRACKET_CLOSE, pos)); break;
				case ':': tokens.push_back(Token(Token::TOKEN_COLON, pos)); break;
				case ',': tokens.push_back(Token(Token::TOKEN_COMMA, pos)); break;
				case '+': tokens.push_back(Token(Token::TOKEN_OP_ADD, pos)); break;
				case '-': tokens.push_back(Token(Token::TOKEN_OP_NEG, pos)); break;
				case '*': tokens.push_back(Token(Token::TOKEN_OP_MULT, pos)); break;
				case '/': tokens.push_back(Token(Token::TOKEN_OP_DIV, pos)); break;
				case '%': tokens.push_back(Token(Token::TOKEN_OP_MOD, pos)); break;
				case '|': tokens.push_back(Token(Token::TOKEN_OP_BIT_OR, pos)); break;
				case '^': tokens.push_back(Token(Token::TOKEN_OP_BIT_XOR, pos)); break;
				case '&': tokens.push_back(Token(Token::TOKEN_OP_BIT_AND, pos)); break;
				case '~': tokens.push_back(Token(Token::TOKEN_OP_BIT_NOT, pos)); break;
				
				// special case for comment
				case '#':
				{
					// check if it's a comment block #* ... *#
					if (source.peek() == '*')
					{
						// comment block
						// record position of the begining
						SourcePos begin(pos);
						// move forward by 2 characters then search for the end
						int step = 2;
						while ((step > 0) || (c != '*') || (source.peek() != '#'))
						{
							if (step)
								step--;

							if (c == '\t')
								pos.column += tabSize;
							else if (c == '\n')
							{
								pos.row++;
								pos.column = 0;
							}
							else
								pos.column++;
							c = source.get();
							pos.character++;
							if (source.eof())
							{
								// EOF -> unbalanced block
								throw Error(begin, "Unbalanced comment block.");
							}
						}
						// fetch the #
						c = source.get();
						pos.column++;
						pos.character++;
					}
					else
					{
						// simple comment
						while ((c != '\n') && (c != '\r') && (!source.eof()))
						{
							if (c == '\t')
								pos.column += tabSize;
							else
								pos.column++;
							c = source.get();
							pos.character++;
						}
						if (c == '\n')
						{
							pos.row++;
							pos.column = 0;
						}
						else if (c == '\r')
							pos.column = 0;
					}
				}
				break;
				
				// cases that require one character look-ahead
				case '!':
					if (source.peek() == '=')
					{
						tokens.push_back(Token(Token::TOKEN_OP_NOT_EQUAL, pos));
						source.get();
						pos.column++;
						pos.character++;
					}
					else
						throw Error(pos, "syntax error");
				break;
				
				case '=':
					if (source.peek() == '=')
					{
						tokens.push_back(Token(Token::TOKEN_OP_EQUAL, pos));
						source.get();
						pos.column++;
						pos.character++;
					}
					else
						tokens.push_back(Token(Token::TOKEN_ASSIGN, pos));
				break;
				
				case '<':
					if (source.peek() == '<')
					{
						tokens.push_back(Token(Token::TOKEN_OP_SHIFT_LEFT, pos));
						source.get();
						pos.column++;
						pos.character++;
					}
					else if (source.peek() == '=')
					{
						tokens.push_back(Token(Token::TOKEN_OP_SMALLER_EQUAL, pos));
						source.get();
						pos.column++;
						pos.character++;
					}
					else
						tokens.push_back(Token(Token::TOKEN_OP_SMALLER, pos));
				break;
				
				case '>':
					if (source.peek() == '>')
					{
						tokens.push_back(Token(Token::TOKEN_OP_SHIFT_RIGHT, pos));
						source.get();
						pos.column++;
						pos.character++;
					}
					else if (source.peek() == '=')
					{
						tokens.push_back(Token(Token::TOKEN_OP_BIGGER_EQUAL, pos));
						source.get();
						pos.column++;
						pos.character++;
					}
					else
						tokens.push_back(Token(Token::TOKEN_OP_BIGGER, pos));
				break;
				
				// cases that require to look for a while
				default:
				{
					// check first character
					if (!std::isalnum(c) && (c != '_'))
						throw Error(pos, "identifiers must begin with _ and an alphanumeric character");
					
					// get a string
					std::string s;
					s += c;
					wchar_t nextC = source.peek();
					int posIncrement = 0;
					while ((source.good()) && (std::isalnum(nextC) || (nextC == '_') || (nextC == '.')))
					{
						s += nextC;
						source.get();
						posIncrement++;
						nextC = source.peek();
					}
					
					// we now have a string, let's check what it is
					if (std::isdigit(s[0]))
					{
						// check if hex or binary
						if ((s.length() > 1) && (s[0] == '0') && (!std::isdigit(s[1])))
						{
							// check if we have a valid number
							if (s[1] == 'x')
							{
								for (unsigned i = 2; i < s.size(); i++)
									if (!std::isxdigit(s[i]))
										throw Error(pos, "error in hexadecimal number");
							}
							else if (s[1] == 'b')
							{
								for (unsigned i = 2; i < s.size(); i++)
									if ((s[i] != '0') && (s[i] != '1'))
										throw Error(pos, "error in binary number");
							}
							else
								throw Error(pos, "error in number, invalid base");
							
						}
						else
						{
							// check if we have a valid number
							for (unsigned i = 1; i < s.size(); i++)
								if (!std::isdigit(s[i]))
									throw Error(pos, "error in number");
						}
						tokens.push_back(Token(Token::TOKEN_INT_LITERAL, pos, s));
					}
					else
					{
						// check if it is a known keyword
						if (s == "when")
							tokens.push_back(Token(Token::TOKEN_STR_when, pos));
						else if (s == "emit")
							tokens.push_back(Token(Token::TOKEN_STR_emit, pos));
						else if (s == "for")
							tokens.push_back(Token(Token::TOKEN_STR_for, pos));
						else if (s == "in")
							tokens.push_back(Token(Token::TOKEN_STR_in, pos));
						else if (s == "step")
							tokens.push_back(Token(Token::TOKEN_STR_step, pos));
						else if (s == "while")
							tokens.push_back(Token(Token::TOKEN_STR_while, pos));
						else if (s == "do")
							tokens.push_back(Token(Token::TOKEN_STR_do, pos));
						else if (s == "if")
							tokens.push_back(Token(Token::TOKEN_STR_if, pos));
						else if (s == "then")
							tokens.push_back(Token(Token::TOKEN_STR_then, pos));
						else if (s == "else")
							tokens.push_back(Token(Token::TOKEN_STR_else, pos));
						else if (s == "elseif")
							tokens.push_back(Token(Token::TOKEN_STR_elseif, pos));
						else if (s == "end")
							tokens.push_back(Token(Token::TOKEN_STR_end, pos));
						else if (s == "var")
							tokens.push_back(Token(Token::TOKEN_STR_var, pos));
						else if (s == "call")
							tokens.push_back(Token(Token::TOKEN_STR_call, pos));
						else if (s == "sub")
							tokens.push_back(Token(Token::TOKEN_STR_sub, pos));
						else if (s == "callsub")
							tokens.push_back(Token(Token::TOKEN_STR_callsub, pos));
						else if (s == "onevent")
							tokens.push_back(Token(Token::TOKEN_STR_onevent, pos));
						else if (s == "abs")
							tokens.push_back(Token(Token::TOKEN_STR_abs, pos));
						else if (s == "or")
							tokens.push_back(Token(Token::TOKEN_OP_OR, pos));
						else if (s == "and")
							tokens.push_back(Token(Token::TOKEN_OP_AND, pos));
						else if (s == "not")
							tokens.push_back(Token(Token::TOKEN_OP_NOT, pos));
						else
							tokens.push_back(Token(Token::TOKEN_STRING_LITERAL, pos, s));
					}
					
					pos.column += posIncrement;
					pos.character += posIncrement;
				}
				break;
			} // switch (c)
		} // while (source.good())
		
		tokens.push_back(Token(Token::TOKEN_END_OF_STREAM, pos));
	}
	
	//! Debug print of tokens
	void Compiler::dumpTokens(std::ostream &dest) const
	{
		for (unsigned i = 0; i < tokens.size(); i++)
			dest << tokens[i].toString() << std::endl;
	}
}; // Aseba
