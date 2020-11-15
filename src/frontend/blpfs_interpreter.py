from blpfs_abi import check_fd_open, read, remove, size, write
from blpfs_parser import APICall, Parser, Program
from blpfs_lexer import Lexer, TokenType

APIList = [ 'print', 'read', 'write', 'remove', 'size', 'copy', 'append' ]

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
    
    def __init__(self, parser: Parser, conn: int):
        self.parser = parser
        self.conn = conn

        if not check_fd_open(self.conn):
            raise Exception('Something wrong. ')

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
        try:
            return self.call(node.api_name, param_values)
        except:
            self.error()

    def call(self, api_name, params):
        method_name = 'api_' + api_name
        api = getattr(self, method_name, self.generic_call)
        return api(params)

    def generic_call(self, node):
        raise Exception(f'No visit_{type(node).__name__} method')

    def api_print(self, params) -> int:
        if len(params) != 1:
            self.error()
        if isinstance(params[0], int):
            print(params[0], end='')
        elif isinstance(params[0], bytearray):
            print(params[0].decode('utf-8'), end='')
        return 0

    def api_read(self, params) -> bytearray:
        if len(params) != 3:
            self.error()
        if not isinstance(params[0], bytearray):
            self.error()
        if not isinstance(params[1], int):
            self.error()
        if not isinstance(params[2], int):
            self.error()
        
        return read(self.conn, params[0], params[1], params[2])

    def api_write(self, params) -> int:
        if len(params) != 3:
            self.error()
        if not isinstance(params[0], bytearray):
            self.error()
        if not isinstance(params[1], bytearray):
            self.error()
        if not isinstance(params[2], int):
            self.error()

        return write(self.conn, params[0], params[2], params[1])

    def api_remove(self, params) -> int:
        if len(params) != 1:
            self.error()
        if not isinstance(params[0], bytearray):
            self.error()

        return remove(self.conn, params[0])
    
    def api_size(self, params) -> int:
        if len(params) != 1:
            self.error()
        if not isinstance(params[0], bytearray):
            self.error()

        return size(self.conn, params[0])

    def api_copy(self, params) -> int:
        if len(params) != 5:
            self.error()
        if not isinstance(params[0], bytearray):
            self.error()
        if not isinstance(params[1], bytearray):
            self.error()
        if not isinstance(params[2], int):
            self.error()
        if not isinstance(params[3], int):
            self.error()
        if not isinstance(params[4], int):
            self.error()

        buf = read(self.conn, params[0], params[2], params[4])
        return write(self.conn, params[1], params[3], buf)

    def api_append(self, params) -> int:
        if len(params) != 2:
            self.error()
        if not isinstance(params[0], bytearray):
            self.error()
        if not isinstance(params[1], bytearray):
            self.error()

        filesize = size(self.conn, params[0])
        return write(self.conn, params[0], filesize, params[1])


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
        