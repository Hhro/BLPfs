#!/usr/bin/env python

import argparse
from blpfs_parser import Parser
from blpfs_lexer import Lexer
from blpfs_interpreter import Interpreter

DEFAULT_FD = 3

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='blpfs VM entrypoint')
    parser.add_argument('code', type=str, nargs='?', help='Code to execute. If none, show prompt. ')

    args = parser.parse_args()
    code = args.code
    if code is None:
        while True:
            line = input('> ')
            if line == 'exit':
                break
            
            lexer = Lexer(line)
            parser = Parser(lexer)
            interpreter = Interpreter(parser, DEFAULT_FD)
            interpreter.interpret()
    else:
        lexer = Lexer(code)
        parser = Parser(lexer)
        interpreter = Interpreter(parser, DEFAULT_FD)
        interpreter.interpret()
        