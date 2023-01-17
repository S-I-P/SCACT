from re import findall
#get starting row, column and ending row, column from clang sourceRange
def getLocation(sourceRange):
    #sourceRange format
    #</path/to/file/filename:(rowNumber):(columnNumber), col:columnNumber> -->> same row or diff row-->>
    #</path/to/file/filename:(rowNumber):(columnNumber), line:(rowNumber):(columnNumber)>
    #spelling included-->
    #</path/to/file/filename:(rowNumber):(columnNumber) <Spelling=line:(rowNumber):(columnNumber)>, ...>
    #</path/to/file/filename:(rowNumber):(columnNumber), ... <Spelling=line:(rowNumber):(columnNumber)>>
    #... --> col:columnNumber or line:(rowNumber):(columnNumber)
    #<Spelling= can also be /path/to/file/filename:(rowNumber):(columnNumber) --> filename not main source file
    #Or <Spelling=col:columnNumber> --> same line
    
    srcRange = sourceRange
    #regex <Spelling=..>
    spellingRegex = r"<Spelling=.+?:[0-9]+>"
    spellingStrs = findall(spellingRegex, sourceRange)
    spellings = []
    for spellingStr in spellingStrs:
        srcRange = srcRange.replace(spellingStr, "")
        if spellingStr.startswith("<Spelling=line"):
            rowSpl,colSpl = spellingStr[:-1].split(":")[-2:]
            spellings.append([int(rowSpl), int(colSpl)])
    # after removing spelling part the last ',' separates begin and end location
    splitIdx = srcRange.rfind(",")
    beginLoc = srcRange[:splitIdx].strip()
    #get start row and column
    idx = beginLoc.rfind(":")
    col1 = int(beginLoc[idx+1:])
    beginLoc = beginLoc[:idx]
    row1 = int(beginLoc[beginLoc.rfind(':')+1:])
    #get end row and column
    endLoc = srcRange[splitIdx+1:-1].strip()
    idx = endLoc.rfind(':')
    col2 = int(endLoc[idx+1:])
    endLoc = endLoc[:idx]
    idx = endLoc.rfind(':')
    #same row col:(columnNumber); diff row line:(rowNumber):(columnNumber)
    if idx==-1:
        row2 = row1
    else:
        row2 = int(endLoc[idx+1:])

    return [row1, col1, row2, col2], spellings

#get information from variable declaration
def variable(lines):
    #first line is the name of the variable
    varName = lines[0]
    #second line is the sourceRange
    srcRange = lines[1]
    if srcRange == "invalid SourceRange":
        location = []
    else:
        location = getLocation(srcRange)
    #third line is variable type
    varType = lines[2]
    #if anonymous struct/union varType has format: "(anonymous struct at /path/filename:line:col)"
    anonRegex = r"\(anonymous .+? at .+?\)"
    anonStrs = findall(anonRegex, varType)
    anons = []
    for anonStr in anonStrs:
        varType = varType.replace(anonStr, "")
        anons.append(anonStr)
    varType = varType.split()
    if varType[0]=="SC_None":
        varType = varType[1:]
    return (varName, location, varType, anons)

#get information from function declaration
def function(lines):
    length = len(lines)
    #first line is the name of the function
    funcName = lines[0].strip()
    #second line is the sourceRange
    srcRange = lines[1]
    if srcRange == "invalid SourceRange":
        return None
    location = getLocation(srcRange)
    #third line is function return type
    returnType = lines[2].split()
    if returnType[0]=="SC_None":
        returnType = returnType[1:]
    #fourth line number of parameters of the function
    numParam = int(lines[3])
    #afterwards each line has the format: parameter name, paramter type 
    params = {}
    for i in range(numParam):
        if lines[4+i][0] == ' ':
            continue
        param = lines[4+i].split()
        if params.get(param[0]) is None:
            params[param[0]] = []
        if param[1]=="SC_None":
            params[param[0]].append(param[2:])
        else:
            params[param[0]].append(param[1:])

    return funcName, location, returnType, numParam, params

#check if a variable was declared within a function
def varInFunc(varLoc, funcLoc):
    if len(varLoc)==0:
        return False
    varRowCol = varLoc[0]
    funcRowCol = funcLoc[0]
    #row not contained
    if varRowCol[0]<funcRowCol[0] or varRowCol[2]>funcRowCol[2]:
        return False
    #row same column not contained
    if varRowCol[0]==funcRowCol[0] and varRowCol[1]>=funcRowCol[1]:
        if varRowCol[2]==funcRowCol[2] and varRowCol[3]>funcRowCol[3]:
            return False
    return True

#add variables to container function
def combine(functions, variables):
    for k,v in functions.items():
        for fi in range(len(v)):
            for vi in range(len(variables)):
                var = variables[vi]
                if varInFunc(var[1], v[fi]['location']):
                    functions[k][fi]['variables'].append(vi)
    return functions
'''
return function and variable info from the clang output
format:
//function:
    <function>
    function_name
    sourcerange
    num parameters(1, 2, ...N)
    return type
    param1 type
    param2 type
    ...
    paramN type
//variable:
    <variable>
    variable name
    sourcerange
    type
'''

def getIdentifierData(lines):
    unit = []
    variables = []
    functions = {}
    length = len(lines)
    if length==0:
        return functions, variables
    if lines[0]=="<function>":
        isFunction = True
    elif lines[0]=="<variable>":
        isFunction = False
    else:
        print("Error Identifier Format")
        return None, None
    idx=1
    while idx<length:
        # new declaration begins
        if lines[idx]=="<function>" or lines[idx]=="<variable>":
            # previous declaration was a function
            if isFunction:
                func = function(unit)
                if functions.get(func[0]) is None:
                    functions[func[0]] = []
                functions[func[0]].append( {
                    'location':func[1], 'return':func[2],
                    'num':func[3], 'params':func[4],
                    'variables':[]
                } )
            # previous declaration was a variable
            else:
                var = variable(unit)
                variables.append(var)
            if lines[idx]=="<function>":
                isFunction = True
            else:
                isFunction = False
            unit = []
        # continue with current declaration
        else:
            unit.append(lines[idx])
        idx += 1
    if isFunction:
        func = function(unit)
        if functions.get(func[0]) is None:
            functions[func[0]] = []
        functions[func[0]].append( {
            'location':func[1], 'return':func[2],
            'num':func[3], 'params':func[4],
            'variables':[]
        } )
    else:
        var = variable(unit)
        variables.append(var)
    # finally check which functions contain which variables
    functions = combine(functions, variables)
    return functions, variables