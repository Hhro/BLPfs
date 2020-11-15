from typing import List
from blpfs_lexer import Lexer, Token, TokenType

class AST(object):
    pass

class Num(AST):
    def __init__(self, token: Token):
        self.token = token
        self.value = token.value

class String(AST):
    def __init__(self, token: Token):
        self.token = token
        self.value = token.value

class Identifier(AST):
    def __init__(self, token: Token):
        self.token = token
        self.value = token.value

class APICall(AST):
    def __init__(self, api_name: Identifier, actual_params: List[AST], token: Token):
        self.api_name = api_name
        self.actual_params = actual_params
        self.token = token

class Program(AST):
    def __init__(self):
        self.children = []

class Parser(object):
    def __init__(self, lexer: Lexer):
        self.lexer = lexer
        self.current_token = self.lexer.get_next_token()

    def error(self):
        raise Exception('Invalid syntax. ')

    def eat(self, ttype: TokenType):
        if self.current_token.ttype == ttype:
            self.current_token = self.lexer.get_next_token()
        else:
            print(self.current_token, ttype)
            self.error()

    def factor(self):
        token = self.current_token
        if token.ttype == TokenType.INTEGER:
            self.eat(TokenType.INTEGER)
            return Num(token)
        elif token.ttype == TokenType.STRING:
            self.eat(TokenType.STRING)
            return String(token)

    def term(self):
        node = self.factor()
        return node

    def expr(self):
        if self.current_token.ttype == TokenType.ID and self.lexer.current_char == '(':
            node = self.apicall()
        else:
            node = self.term()
        return node

    def apicall(self):
        token = self.current_token
        api_name = self.current_token.value
        self.eat(TokenType.ID)
        self.eat(TokenType.LPAREN)
        actual_params = []
        if self.current_token.ttype != TokenType.RPAREN:
            node = self.expr()
            actual_params.append(node)
        while self.current_token.ttype == TokenType.COMMA:
            self.eat(TokenType.COMMA)
            node = self.expr()
            actual_params.append(node)
        
        self.eat(TokenType.RPAREN)
        node = APICall(api_name, actual_params, token)
        return node

    def statement(self):
        # TODO: if implement assign -> apply here
        return self.expr()

    def statement_list(self):
        node = self.statement()
        results = [node]

        while self.current_token.ttype == TokenType.SEMI:
            self.eat(TokenType.SEMI)
            results.append(self.statement())

        if self.current_token.ttype != TokenType.EOF:
            self.error()

        return results

    def program(self):
        nodes = self.statement_list()
        self.eat(TokenType.EOF)
        
        root = Program()
        for node in nodes:
            root.children.append(node)
        return root

    def parse(self):
        return self.program()

if __name__ == "__main__":
    from pprint import pprint
    while True:
        line = input('> ')
        if line == 'exit':
            break
        
        lexer = Lexer(line)
        parser = Parser(lexer)
        pprint(vars(parser.parse()))
        