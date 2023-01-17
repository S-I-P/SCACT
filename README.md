# SCACT
Source Code Analyzer Clang Tool.\
SCACT is a set of four clang tools that returns the following information about C/C++ source code:

* Lexer-tokens
* Abstract Syntax Tree(AST)
* Functions: Return type, parameters(names and types)
* Variable: Names with types
* Class/struct/union: Fields and methods
* Enum: fields
* Typedef: name and underlying type

Returns the source range in the source code for everything except the AST as well.


## Prerequisites

1. llvm/clang\
The codes link against clang libraries in version [8](https://github.com/llvm/llvm-project/tree/release/8.x) (8.0.0/8.0.1). Other versions of clang/llvm libraries may not work.
1. CMake version 3+
1. python 3(optional)

## Build

1. [Building and Running Clang](https://clang.llvm.org/get_started.html)\
While building clang use -DLLVM_ENABLE_PROJECTS=”clang;clang-tools-extra”.

1. After building, add the folders from [clang-codes](clang-codes) in llvm-src-dir/clang-tools-extra. In the CMakeLists.txt file in clang-tools-extra add the following lines:
```
add_subdirectory(lexical-tokens)
add_subdirectory(clang-ast)
add_subdirectory(identifier)
add_subdirectory(record)
```

1. make\
make from llvm build directory. The llvm bin directory should have:`lex-tok, clang-ast, idt-info, record-info`

Refer to https://clang.llvm.org/docs/LibASTMatchersTutorial.html to learn more about building tools using LibTooling.


## Components

The four tools:

* #### Lexer tokens
Prints the lexer output tokens with location line by line. The output is equivalent to
```
clang -fsyntax-only -Xclang -dump-raw-tokens source_code.c
```
Usage:
```
/path/to/llvm-bins/lex-tok <sourceFile> -- -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```
Output Format:
```
<beginLocation> <endLocation> token_type symbol
<beginLocation> <endLocation> token_type symbol
...
...
...
eof
```
P.S. This tool relies on the clang lexer function `lexer::findNextToken`, which may miss the very first token. However, adding an extra line to the beginning of the source code file solves this issue.


* #### AST
Prints the AST nodes with id that can be used to build the AST. This tool returns the AST equivalent to:
```
clang -fsyntax-only -Xclang -ast-dump <sourceFile>
```
Only the AST nodes from the main source file(not headers) are output.
Usage:
```
/path/to/llvm-bins/clang-ast <sourceFile> -- -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```
Output Format:
```
1 TranslationUnitDecl
node_id node_name parent_name parent_id
...
```

* #### Identifier
Prints information about functions and variables.
Usage:
```
/path/to/llvm-bins/idt-info <sourceFile> -- -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```
Output Format:
```
    <function>
    function_name
    sourcerange
    num parameters(1, 2, ...N)
    return type
    param1 type
    param2 type
    ...
    paramN type
    <variable>
    variable name
    sourcerange
    type
```

* #### Record
Prints information about enum, typedef, struct, union, class
Usage:
```
/path/to/llvm-bins/record-info <sourceFile> -- -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```
Output Format:
```
//if enum:
    <enum>
    enum_name
    sourcerange
    num fields(1, 2, ...N)
    field1 name
    field2 name
    ...
    fieldN name
//if typedef:
    <typedef>
    typedef name
    underlying type
    sourcerange
//if record(struct/union/class):
    <struct/union/class>
    name
    sourcerange
    num fields(1, 2, ...N)
    field1_name type
    field2_name type
    ...
    fieldN_name type
    <Record>(c)/<CXXRecord>(c++)
    //if CXXRecord:
        num methods(1, 2, ...N)
        <constructor>
        ...
        <constructor>
        <destructor>
        ...
        <destructor>
        <method>
        ...
        .
        .
        .
        ...
        <method>
        * ... -->
        name
        sourcerange
        storage_container static/const/volatile/virtual
        num parameters(1, 2, ...N)
        return type
        param1 type
        param2 type
        ...
        paramN type
```

The text files in [sample folder](sample) contain the output of the tools when run on [sample.cpp](sample/sample.cpp).

## Python

The python codes parse the output of the tools, then structure it, and finally return JSON format. The files: `tokenizer.py, clang_ast.py, identifier.py, record.py` process the output of the corresponding tool. Run "main.py" in python folder for all four tools. Modify path of the tools in lines 20-23 of main.py. Output format:

```
{
    'tokens' : tokens,
    'symbols' : symbols,
    'locations' : locations,
    'unk' : unk,
    'ast' : ast,
    'variables' : variables,
    'functions' : functions,
    'enums' : enums,
    'typedefs' : typedefs,
    'records' : records
}
```
Here, 'tokens', 'symbols', 'locations' are lists of the same length, output of the lex-tok tool, and correspond to the abstract lexer tokens, symbols, and source location, respectively. Each item of location is a list of length 4, representing the start row, start column, end row, and end column. 'unk' is also the output of the lex-tok tool. It is a list of unknown tokens.\
Secondly, the ast format is:
```
{
    "kind": "TranslationUnitDecl",
    "child": {
        "child_id": {
            kind: "node_kind",
            child: {
                ...
            }
        }, ...
    }
    ...
}
```
Then, 'variables', and 'functions' are outputs of the tool idt-info. Here, 'variable' is a list where each item is a list describing a variable. Format:
```
[
    [
        "variable_name",
        [start row, start column, end row, end column],
        [spelling_location],
        [variable_type],
        [anonymous]
    ],...
]
```
The first element is the name of the variable, followed by the source range. The next item is a list of the spelling locations of the variable (e.g., #define). The fourth item is a list describing the variable type. Finally, if the variable is an instance of an anonymous struct, union, etc., the last list includes information regarding this.\
'functions' is a dictionary where the key is the name of the function. The values are lists of dictionaries. Overall format:
```
{
    "function_name" : [
        {
            'location': [[start row, start column, end row, end column], [spelling_location]],
            'return': [return_type],
            'num': N,
            'params': {
                "param1_name": [param1_type], ..., "paramN_name": [paramN_type]
            },
            'variables' : []
        }, ...
    ],...
}
```
Here, 'variables' is a list of integers corresponding to the indices of variables in the list 'variables' that were declared within the scope of this function.\
Finally, 'enums', 'typedefs', and 'records' are outputs of the tool record-info. Format of 'enums' is:
```
{
    'enum_name': [
        {
            'location': [[start row, start column, end row, end column], [spelling_location]],
            'fields': ['field_name',...]
        },...
    ],...
}
```
Format of 'typedefs' is:
```
{
    'typedef_name': [
        {
            'type': [underlying_type],
            'location': [[start row, start column, end row, end column], [spelling_location]]
        }, ...
    ],...
}
```
Format of 'records' is:
```
{
    'name': [
        {
            'type': "class/struct/union",
            'location': [[start row, start column, end row, end column], [spelling_location]],
            'fields': {
                'field_name': [
                    [
                        [type]
                    ],...
                ],...
            },
            'methods': {
                'constructor': {*},
                'destructor': {*},
                'method': {*}
                "* same as function format"
            }
        }, ...
    ],...
}
```

Refer to [sample.json](sample/sample.json) (for source code [sample.cpp](sample/sample.cpp)) to get a better understanding of the JSON output format.