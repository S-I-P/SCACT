#include "llvm/Support/CommandLine.h"

#include "clang/Basic/SourceManager.h"
#include "clang/Basic/FileManager.h"

#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/FrontendOptions.h"
#include "clang/Frontend/Utils.h"

#include "clang/Lex/Lexer.h"
#include "clang/Lex/Token.h"

#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTTypeTraits.h"

#include <memory>
#include <vector>
#include <iostream>
#include <typeinfo>


using namespace clang;
using namespace clang::tooling;

static llvm::cl::OptionCategory MyOpts("Ignored");

std::string fileName;

class TraverseAstAction : public clang::ASTFrontendAction {
public:
    virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
    clang::CompilerInstance &Compiler, llvm::StringRef InFile) {

        ASTContext& Context = Compiler.getASTContext();
        SourceManager& srcmgr = Context.getSourceManager();
        FileManager& filemgr = srcmgr.getFileManager();
        const FileEntry* file_entry_ptr = filemgr.getFile(fileName);
        SourceLocation loc = srcmgr.translateFileLineCol(file_entry_ptr, 1, 1);
        LangOptions langOpts = Context.getLangOpts();

        using namespace clang::tok;

        while (true) {
            Token *tok = Lexer::findNextToken(loc, srcmgr, langOpts).getPointer();
            tok::TokenKind tokKind = tok->getKind();
            std::string tokName = (std::string)tok->getName();
            std::string tokSpell = Lexer::getSpelling(*tok, srcmgr, langOpts);

            const char* keyWd = getKeywordSpelling(tokKind);
            bool isKeyWd = false;

            if (keyWd != nullptr)
                isKeyWd = true;
                


            if (tokName == "eof") {
                std::cout<<"eof\n";
                break;
            }
            std::cout<<tok->getLocation().printToString(srcmgr)<<" ";
            std::cout<<tok->getEndLoc().printToString(srcmgr)<<" ";

            if (isKeyWd)
                std::cout<<tokSpell<<" "<<tokSpell<<"\n";
            else
                std::cout<<tokName<<" "<<tokSpell<<"\n";

            

            loc = tok->getLocation();
        }
        return 0;
    }
};

int main(int argc, const char ** argv)
{
    fileName = argv[1];
    CommonOptionsParser opt_prs(argc, argv, MyOpts);
    ClangTool tool(opt_prs.getCompilations(), opt_prs.getSourcePathList());
    int result = tool.run(newFrontendActionFactory<TraverseAstAction>().get());
    return result;
}