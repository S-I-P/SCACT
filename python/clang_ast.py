# recursively build AST and return as dict
def buildtree(nodeid, nodename, nodelist):
    subast = {
        'kind' : nodename
    }
    child = {}
    for n in nodelist:
        if (n[2] == nodeid) and (n[3] == nodename):
            child[n[0]] = buildtree(n[0], n[1], nodelist)
    subast['child'] = child
    return subast
'''
clang ast output format:
node_id node_name parent_name parent_id
1st line root node
'''
def getAST(astNodes):
    root = astNodes[0].split()
    rootID = root[0]
    rootName = root[1]
    nodelist = []
    
    for node in astNodes[1:]:
        '''
        i1 = node.find(' ')
        i2 = node.find(' ', i1+1)
        i3 = node.find(' ', i2+1)
        nodeid = node[:i1]
        nodename = node[i1+1:i2]
        parentname = node[i2+1:i3]
        parentid = node[i3+1:]
        '''
        nodeid, nodename, parentname, parentid = node.split()
        nodelist.append((nodeid, nodename, parentid, parentname))
    return buildtree(rootID, rootName, nodelist)