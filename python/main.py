import subprocess
import os
import json
import sys
from tokenizer import tokenize
from clang_ast import getAST
from identifier import getIdentifierData
from record import getRecordData

def runCmd(cmd):
    proc = subprocess.Popen(cmd,
        stdout = subprocess.PIPE,
        stderr = subprocess.PIPE,
    )
    stdout, stderr = proc.communicate()

    return proc.returncode, stdout, stderr

clang_tools = {
    "tokenizer": "/path/to/llvm/build/bin/lex-tok",
    "ast": "/path/to/llvm/build/bin/clang-ast",
    "identifier": "/path/to/llvm/build/bin/idt-info",
    "record": "/path/to/llvm/build/bin/record-info"
}

if len(sys.argv)<2:
    print("Usage: python main.py <filepath>")
    exit(0)

fname = sys.argv[1]
outfile = fname[:fname.rfind('.')] + '.json'

cmd = []
cmd.append("")
cmd.append(fname)
cmd.append("--")
cmd.append("-DCMAKE_EXPORT_COMPILE_COMMANDS=ON")


# tokenize
cmd[0] = clang_tools['tokenizer']
out = runCmd(cmd)[1].decode('utf-8')
tokens, symbols, pos, unk = tokenize(fname, out.splitlines()[:-1])

# AST
cmd[0] = clang_tools["ast"]
out = runCmd(cmd)[1].decode('utf-8')
ast = getAST(out.splitlines())

#identifier
cmd[0] = clang_tools['identifier']
out = runCmd(cmd)[1].decode('utf-8').splitlines()
functions, variables = getIdentifierData(out)

#record
cmd[0] = clang_tools['record']
out = runCmd(cmd)[1].decode('utf-8').splitlines()
enums, typedefs, records = getRecordData(out)

out = {
    'tokens' : tokens,
    'symbols' : symbols,
    'locations' : pos,
    'unk' : unk,
    'ast' : ast,
    'variables' : variables,
    'functions' : functions,
    'enums' : enums,
    'typedefs' : typedefs,
    'records' : records
}
with open(outfile, 'w') as f:
    json.dump(out, f)