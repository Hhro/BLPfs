from enum import Enum
from string import ascii_letters, digits

simple_escape_sequence = {
    'n': '\n',
    't': '\t',
    'v': '\v',
    'b': '\b',
    'r': '\r',
    'f': '\f',
    'a': '\a',
    '\\': '\\',
    '?': '?',
    '\'': '\'',
    '"': '\"',
}

class TokenType(Enum):
    INTEGER = 'INTEGER'
    STRING = 'STRING'
    ID = 'IDENTIFIER'
    SEMI = 'SEMI'
    LPAREN = 'LPAREN'
    RPAREN = 'RPAREN'
    COMMA = 'COMMA'
    EOF = 'EOF'

class Token(object):
    def __init__(self, ttype: TokenType, value):
        self.ttype = ttype
        self.value = value

    def __str__(self):
        return f'Token({self.ttype}, {self.value})'
    
    def __repr__(self):
        return self.__str__()

class Lexer(object):
    def __init__(self, text: str):
        self.text = text
        self.pos = 0
        self.current_char = self.text[self.pos]
    
    def error(self):
        print(self.current_char)
        raise Exception('Invalid character. ')

    def advance(self):
        self.pos += 1
        if self.pos > len(self.text) - 1:
            self.current_char = None
        else:
            self.current_char = self.text[self.pos]
    
    def peek(self):
        peek_pos = self.pos + 1
        if peek_pos > len(self.text) - 1:
            return None
        else:
            return self.text[peek_pos]
        
    def skip_whitespace(self):
        while self.current_char is not None and self.current_char.isspace():
            self.advance()

    def integer(self) -> int:
        result = ''
        while self.current_char is not None and self.current_char.isdigit():
            result += self.current_char
            self.advance()
        return int(result)

    def hexadecimal_escape_sequence(self) -> int:
        result = ''
        while self.current_char is not None and self.current_char in '0123456789abcdefABCDEF':
            result += self.current_char
            self.advance()
        if len(result) > 2 or len(result) == 0:
            self.error()
        return int(result, 16)

    def hexadecimal_escape_sequence(self) -> int:
        result = ''
        while self.current_char is not None and self.current_char in '0123456789abcdefABCDEF':
            result += self.current_char
            self.advance()
        if len(result) == 0:
            self.error()
        return int(result, 16)

    def octal_escape_sequence(self) -> int:
        result = ''
        while self.current_char is not None and self.current_char in '01234567':
            result += self.current_char
            self.advance()
        if len(result) > 3 or len(result) == 0:
            self.error()
        return int(result, 8)

    def escape_sequence(self) -> int:
        if self.current_char != '\\':
            self.error()
        self.advance()
        if self.current_char in simple_escape_sequence.keys():
            result = ord(simple_escape_sequence[self.current_char])
            self.advance()
            return result
        elif self.current_char == 'x':
            self.advance()
            return self.hexadecimal_escape_sequence()
        else:
            return self.octal_escape_sequence()

    def string(self) -> bytearray:
        result = bytearray()
        if self.current_char != '"':
            self.error()
        self.advance()
        while self.current_char is not None and self.current_char in ascii_letters + digits + '"\\':
            if self.current_char == '"':
                self.advance()
                return result
            elif self.current_char == '\\':
                result.append(self.escape_sequence())
            else:
                result.append(ord(self.current_char))
                self.advance()
        self.error()
    
    def _id(self) -> str:
        result = ''
        while self.current_char is not None and self.current_char.isalnum():
            result += self.current_char
            self.advance()
        return result
        
    def get_next_token(self) -> Token:
        while self.current_char is not None:
            if self.current_char.isspace():
                self.skip_whitespace()
                continue
            
            if self.current_char.isdigit():
                return Token(TokenType.INTEGER, self.integer())

            if self.current_char == '"':
                return Token(TokenType.STRING, self.string())

            if self.current_char.isalpha():
                return Token(TokenType.ID, self._id())

            if self.current_char == '(':
                self.advance()
                return Token(TokenType.LPAREN, '(')

            if self.current_char == ')':
                self.advance()
                return Token(TokenType.RPAREN, ')')

            if self.current_char == ';':
                self.advance()
                return Token(TokenType.SEMI , ';')

            if self.current_char == ',':
                self.advance()
                return Token(TokenType.COMMA , ',')

            self.error()
        return Token(TokenType.EOF, None)
    
if __name__ == "__main__":
    while True:
        line = input('> ')
        if line == 'exit':
            break
        
        lexer = Lexer(line)
        while True:
            token = lexer.get_next_token()
            print(token)
            if token.ttype == TokenType.EOF:
                break