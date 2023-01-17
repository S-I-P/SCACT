# keywords that are printed as raw_identifier in clang lexer output
keywords = [
    'auto'
    ,'break'
    ,'case'
    ,'char'
    ,'const'
    ,'continue'
    ,'default'
    ,'do'
    ,'double'
    ,'else'
    ,'enum'
    ,'extern'
    ,'float'
    ,'for'
    ,'goto'
    ,'if'
    ,'inline'
    ,'int'
    ,'long'
    ,'register'
    ,'restrict'
    ,'return'
    ,'short'
    ,'signed'
    ,'sizeof'
    ,'static'
    ,'struct'
    ,'switch'
    ,'typedef'
    ,'union'
    ,'unsigned'
    ,'void'
    ,'volatile'
    ,'while'
    ,'_Alignas'
    ,'_Alignof'
    ,'_Atomic'
    ,'_Bool'
    ,'_Complex'
    ,'_Generic'
    ,'_Imaginary'
    ,'_Noreturn'
    ,'_Static_assert'
    ,'_Thread_local'
    ,'__func__'
    ,'__objc_yes'
    ,'__objc_no'
    ,'asm'
    ,'bool'
    ,'catch'
    ,'class'
    ,'const_cast'
    ,'delete'
    ,'dynamic_cast'
    ,'explicit'
    ,'export'
    ,'false'
    ,'friend'
    ,'mutable'
    ,'namespace'
    ,'new'
    ,'operator'
    ,'private'
    ,'protected'
    ,'public'
    ,'reinterpret_cast'
    ,'static_cast'
    ,'template'
    ,'this'
    ,'throw'
    ,'true'
    ,'try'
    ,'typename'
    ,'typeid'
    ,'using'
    ,'virtual'
    ,'wchar_t'
    ,'alignas'
    ,'alignof'
    ,'char16_t'
    ,'char32_t'
    ,'constexpr'
    ,'decltype'
    ,'noexcept'
    ,'nullptr'
    ,'static_assert'
    ,'thread_local'
    ,'concept'
    ,'requires'
    ,'co_await'
    ,'co_return'
    ,'co_yield'
    ,'module'
    ,'import'
    ,'char8_t'
    ,'consteval'
    ,'constinit'
    ,'_Float16'
    ,'_Accum'
    ,'_Fract'
    ,'_Sat'
    ,'_Decimal32'
    ,'_Decimal64'
    ,'_Decimal128'
    ,'__null'
    ,'__alignof'
    ,'__attribute'
    ,'__builtin_choose_expr'
    ,'__builtin_offsetof'
    ,'__builtin_FILE'
    ,'__builtin_FUNCTION'
    ,'__builtin_LINE'
    ,'__builtin_COLUMN'
    ,'__builtin_va_arg'
    ,'__extension__'
    ,'__float128'
    ,'__imag'
    ,'__int128'
    ,'__label__'
    ,'__real'
    ,'__thread'
    ,'__FUNCTION__'
    ,'__PRETTY_FUNCTION__'
    ,'__auto_type'
    ,'typeof'
    ]

def getRowCol(loc):
    # index of last : in location
    # searching from end because there may be more than 2 : in location but the last 2 are related to row and column
    splitRC = loc.rfind(':')
    # start column
    col = int(loc[splitRC+1:])
    # remove column location part
    loc = loc[:splitRC]
    # now row is after the last :
    splitRC = loc.rfind(':')
    row = int(loc[splitRC+1:])
    return row, col


#clang lexer output format: startlocation(filename:row:column) endlocation(same as start) token symbol
def tokenize(fname, rawTokens):
    #print(rawTokens)
    tokens = []
    symbols = []
    pos = []
    unk = []
    for rawTok in rawTokens:
        #if multiline string_literal then filename not in output
        if rawTok.find(fname)==-1:
            if len(symbols)>0 and tokens[-1]=="string_literal":
                symbols[-1] += "\n"+rawTok
            else:
                unk.append(rawTok)
            continue
        #discard filepath from rawtok
        rawTok = rawTok.replace(fname, "", 2)
        # index of first whitespace
        splitAt = rawTok.find(' ')
        # startlocation
        beginLoc = rawTok[:splitAt]
        row1, col1 = getRowCol(beginLoc)
        # discard startlocation part from rawTok
        rawTok = rawTok[splitAt+1:]
        splitAt = rawTok.find(' ')
        # endlocation
        endLoc = rawTok[:splitAt]
        row2, col2 = getRowCol(endLoc)
        # discard endlocation part from rawTok
        rawTok = rawTok[splitAt+1:]
        splitAt = rawTok.find(' ')
        tok1 = rawTok[:splitAt]
        tok2 = rawTok[splitAt+1:]
        #print(row1, col1, row2, col2, tok1, tok2)
        pos.append([row1, col1, row2, col2])
        # raw identifier token(may be keywords in the list above or)
        if (tok1 == 'raw_identifier'):
            if (tok2 in keywords):
                tokens.append(tok2)
                symbols.append(tok2)
            else:
                tokens.append('identifier')
                symbols.append(tok2)
        # other token type
        else:
            tokens.append(tok1)
            symbols.append(tok2)
    ##print(tokens)
    ##print(symbols)
    return tokens, symbols, pos, unk