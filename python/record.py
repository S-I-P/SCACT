from identifier import getLocation, function

#get information from enum declaration
def enum(lines):
    #first line is the name of the enum
    enumName = lines[0]
    #second line is the sourceRange
    srcRange = lines[1]
    if srcRange == "invalid SourceRange":
        location = []
    else:
        location = getLocation(srcRange)
    #third line is number of fields followed by name of fields
    fields = []
    numFields = int(lines[2])
    for i in range(numFields):
        fields.append(lines[i+3])
    return (enumName, location, fields)

#get information from typedef declaration
def typedef(lines):
    #first line is the name of the typedef
    typedefName = lines[0]
    #second line is underlying type
    undType = lines[1].split()
    #third line is the sourceRange
    srcRange = lines[2]
    if srcRange == "invalid SourceRange":
        location = []
    else:
        location = getLocation(srcRange)
    return (typedefName, undType, location)

#get information about method
#format (same as function except methods are separated by newline)
### name
### sourcerange
### storage_container static/const/volatile/virtual
### num parameters(1, 2, ...N)
### return type
### param1 type
### param2 type
### ...
### paramN type
### <blank>
def method(lines):
    length = len(lines)
    idx = 0
    unit = []
    methods = {}
    while idx<length:
        if len(lines[idx]):
            unit.append(lines[idx])
        else:#new method
            method_ = function(unit)
            methodName = method_[0]
            if methods.get(methodName) is None:
                methods[methodName] = []
            methods[methodName].append( {
                    'location':method_[1], 'return':method_[2],
                    'num':method_[3], 'params':method_[4]
                } )
            unit = []
        idx += 1
    return methods

#return constructor/destructor/method lines
def methodUnits(lines, methodType):
    unit = []
    #units between <constructor>...<constructor> or destructor or method
    assert lines[0]==methodType
    idx = 1
    while lines[idx]!=methodType:
        unit.append(lines[idx])
        idx += 1
    return unit, idx

#get information from record declaration
def record(lines):
    length = len(lines)
    #first line is the name
    recordName = lines[0]
    #second line is the sourceRange
    srcRange = lines[1]
    if srcRange == "invalid SourceRange":
        location = []
    else:
        location = getLocation(srcRange)
    #third line is number of fields followed by name of fields
    fields = {}
    numFields = int(lines[2])
    for i in range(numFields):
        currField = lines[i+3].split()
        if fields.get(currField[0]) is None:
            fields[currField[0]] = []
        fields[currField[0]].append(currField[1:])
    #next line Record for c records; CXXRecord for c++
    idx = len(fields)+3
    methods = {}
    if lines[idx]=="CXXRecord":
        methodLines = lines[idx+1:]
        numMethods = int(methodLines[0])
        if numMethods:
            methodLines = methodLines[1:]
            # first constructor unit
            unit, idx = methodUnits(methodLines, '<constructor>')
            methods['constructor'] = method(unit)
            #next unit
            methodLines = methodLines[idx+1:]
            # second destructor unit
            unit, idx = methodUnits(methodLines, '<destructor>')
            methods['destructor']  = method(unit)
            #next unit
            methodLines = methodLines[idx+1:]
            # last unit other methods
            unit, idx = methodUnits(methodLines, '<method>')
            methods['method']  = method(unit)
    
    return (recordName, location, fields, methods)

'''
Get information about typedef, struct/union/class/enum
format:
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
    source range
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
    if CXXRecord:
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
    ~~~~~~~~~~
        name
        sourcerange
        storage_container static/const/volatile/virtual
        num parameters(1, 2, ...N)
        return type
        param1 type
        param2 type
        ...
        paramN type
        
    ~~~~~~~~~~
'''

def getRecordData(lines):
    unit = []
    enums = {}
    typedefs = {}
    records = {}
    length = len(lines)
    if length==0:
        return enums, typedefs, records
    #record kinds
    kinds = ["<enum>", "<typedef>", "<struct>", "<union>", "<class>"]
    #flag list for checking unit which kind; idx 0 enum, idx 1 typedef, idx 2 others
    flag = [False, False, False]
    #Record/typedef to flag list index map
    map_ = {
        kinds[0] : 0,
        kinds[1] : 1,
    }
    for i in range(2,5):
        map_[kinds[i]] = 2
    if lines[0] in kinds:
        flag[map_[lines[0]]] = True
    else:
        print("Error Record Format")
        return None, None, None
    idx=1
    while idx<length:
        # new declaration begins
        if lines[idx] in kinds:
            if flag[0]:#prev decl is enum
                enum_ = enum(unit)
                enumName = enum_[0]
                if enums.get(enumName) is None:
                    enums[enumName] = []
                enums[enumName].append({
                    'location' : enum_[1],
                    'fields' : enum_[2]
                })
            elif flag[1]:#prev decl is typedef
                typedef_ = typedef(unit)
                typedefName = typedef_[0]
                if typedefs.get(typedefName) is None:
                    typedefs[typedefName] = []
                typedefs[typedefName].append({
                    'type' : typedef_[1],
                    'location' : typedef_[2]
                })
            elif flag[2]:
                recordType = lines[idx-len(unit)-1][1:-1]#struct/union/class
                record_ = record(unit)
                recordName = record_[0]
                if records.get(recordName) is None:
                    records[recordName] = []
                records[recordName].append({
                    'type' : recordType,
                    'location' : record_[1],
                    'fields' : record_[2],
                    'methods' : record_[3]
                })
            flag = [False, False, False]
            flag[map_[lines[idx]]] = True
            unit = []
        # continue with current declaration
        else:
            unit.append(lines[idx])
        idx += 1
    #last declaration
    if flag[0]:#prev decl is enum
        enum_ = enum(unit)
        enumName = enum_[0]
        if enums.get(enumName) is None:
            enums[enumName] = []
        enums[enumName].append({
            'location' : enum_[1],
            'fields' : enum_[2]
        })
    elif flag[1]:#prev decl is typedef
        typedef_ = typedef(unit)
        typedefName = typedef_[0]
        if typedefs.get(typedefName) is None:
            typedefs[typedefName] = []
        typedefs[typedefName].append({
            'type' : typedef_[1],
            'location' : typedef_[2]
        })
    elif flag[2]:
        recordType = lines[idx-len(unit)-1][1:-1]#struct/union/class
        record_ = record(unit)
        recordName = record_[0]
        if records.get(recordName) is None:
            records[recordName] = []
        records[recordName].append({
            'type' : recordType,
            'location' : record_[1],
            'fields' : record_[2],
            'methods' : record_[3]
        })
    return enums, typedefs, records
