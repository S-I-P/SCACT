/*
Get information about functions and variables
format:
//if function:
    <function>
    function_name
    sourcerange
    num parameters(1, 2, ...N)
    return type
    param1 type
    param2 type
    ...
    paramN type
//else(variable):
    <variable>
    variable name
    sourcerange
    type
*/

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

#include <memory>
#include <vector>
#include <iostream>

using namespace clang;
using namespace clang::tooling;

static llvm::cl::OptionCategory MyOpts("Ignored");

std::string fileName;
// print storage container type
#define stringify( name ) # name
const char* SC[] = 
  {
    stringify(SC_None),
    stringify(SC_Extern),
    stringify(SC_Static),
    stringify(SC_PrivateExtern),
    stringify(SC_Auto),
    stringify(SC_Register)
  };

class customVisitor
  : public RecursiveASTVisitor<customVisitor> {
public:
  explicit customVisitor(ASTContext *Context)
    : Context(Context) {}

    bool VisitDecl(Decl *Declaration) {
        
        clang::SourceManager& mgr = Context->getSourceManager();

        if (mgr.isInMainFile(Declaration->getLocation())) {
            // Function
            if(!strcmp(Declaration->getDeclKindName(), "Function")) {
                // cast to FunctionDecl
                FunctionDecl* func= llvm::dyn_cast<FunctionDecl>(Declaration);
                if(func->isThisDeclarationADefinition()) {
                    std::cout<<"<function>\n"<<func->getNameInfo().getAsString()<<"\n";
                    // Source Range
                    SourceRange srcRange = func->getSourceRange();
                    if (srcRange.isValid()) {
                        std::cout<<srcRange.printToString(mgr)<<"\n";
                    }
                    else {
                        std::cout<<"invalid SourceRange\n";
                    }
                    // print function return type
                    std::cout<<SC[func->getStorageClass()]<<" "<<func->getReturnType().getAsString()<<"\n";                    
                    // print number of paramters and their names, types
                    int numParams = func->getNumParams();
                    std::cout<<numParams<<"\n";
                    for(int i=0; i<numParams; i++) {
                        auto param = func->parameters()[i];
                        std::cout<<param->getNameAsString()<<" "<<param->getOriginalType().getAsString()<<"\n";
                    }
                }
            }
            // Variable
            else if(!strcmp(Declaration->getDeclKindName(), "Var")) {
                // cast to VarDecl
                VarDecl *var = llvm::dyn_cast<VarDecl>(Declaration);
                std::cout<<"<variable>\n"<<var->getNameAsString()<<"\n";
                // Source Range
                SourceRange srcRange = var->getSourceRange();
                if (srcRange.isValid()) {
                    std::cout<<srcRange.printToString(mgr)<<"\n";
                }
                else {
                    std::cout<<"invalid SourceRange\n";
                }
                //print type
                std::cout<<SC[var->getStorageClass()]<<" "<<var->getType().getAsString()<<"\n";
            }
        }
        return true;

    }

private:
    ASTContext *Context;
};

class TraverseAstConsumer : public clang::ASTConsumer {
public:
    explicit TraverseAstConsumer(ASTContext *Context)
        : Visitor(Context) {}

    virtual void HandleTranslationUnit(clang::ASTContext &Context) {
        Visitor.TraverseDecl(Context.getTranslationUnitDecl());
    }
private:
    customVisitor Visitor;
};

class TraverseAstAction : public clang::ASTFrontendAction {
public:
    virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
    clang::CompilerInstance &Compiler, llvm::StringRef InFile) {

        return std::unique_ptr<clang::ASTConsumer>(
            new TraverseAstConsumer(&Compiler.getASTContext()));
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