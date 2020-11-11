from blpfs_parser import APICall, Parser, Program
from blpfs_lexer import Lexer, TokenType

APIList = [ 'print', ]

class NodeVisitor(object):
    def visit(self, node):
        method_name = 'visit_' + type(node).__name__
        visitor = getattr(self, method_name, self.generic_visit)
        return visitor(node)
    
    def generic_visit(self, node):
        raise Exception(f'No visit_{type(node).__name__} method')

class Interpreter(NodeVisitor):
    def error(self):
        raise Exception("Runtime exception. ")
    def __init__(self, parser: Parser):
        self.parser = parser

    def visit_Num(self, node):
        return node.value
    
    def visit_String(self, node):
        return node.value

    def visit_Program(self, node: Program):
        result = 0
        for child in node.children:
            if child is not None:
                result = self.visit(child)
        return result

    def visit_APICall(self, node: APICall):
        if node.api_name not in APIList:
            self.error()
        
        param_values = []
        for param in node.actual_params:
            param_values.append(self.visit(param))
        return self.call(node.api_name, param_values)

    def call(self, api_name, params):
        method_name = 'api_' + api_name
        api = getattr(self, method_name, self.generic_call)
        return api(params)

    def generic_call(self, node):
        raise Exception(f'No visit_{type(node).__name__} method')

    def api_print(self, params):
        if len(params) != 1:
            self.error()
        if isinstance(params[0], int):
            print(params[0], end='')
        elif isinstance(params[0], bytearray):
            print(params[0].decode('utf-8'), end='')
        return 0

    def interpret(self):
        tree = self.parser.parse()
        return self.visit(tree)

if __name__ == "__main__":
    while True:
        line = input('> ')
        if line == 'exit':
            break
        
        lexer = Lexer(line)
        parser = Parser(lexer)
        interpreter = Interpreter(parser)
        print(interpreter.interpret())
        