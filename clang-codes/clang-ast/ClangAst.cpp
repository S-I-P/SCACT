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

class customVisitor
  : public RecursiveASTVisitor<customVisitor> {
public:
  explicit customVisitor(ASTContext *Context)
    : Context(Context) {}

    bool VisitDecl(Decl *Declaration) {
        
        clang::SourceManager& mgr = Context->getSourceManager();
        if (mgr.isInMainFile(Declaration->getLocation())) {
            
            bool notParent = true;
            // create dynamically typed AST node container for working with different types of AST nodes
            ast_type_traits::DynTypedNode currNode = clang::ast_type_traits::DynTypedNode::create(*Declaration);
            
            while (notParent) {

                auto parentsList = Context->getParents(currNode);
                // only 1 parent so the loop runs one time
                for(auto p=parentsList.begin(); p!= parentsList.end(); p++) {
                    //std::cout<<p->getNodeKind().asStringRef().str()<<"\n";
                    auto parDecl = p->get<Decl>();
                    auto parStmt = p->get<Stmt>();
                    if (parDecl!=0) notParent = false;
                    if (parStmt!=0) notParent = false;
                } 

                currNode = *parentsList.begin();

            }
            // print the ID of the current Decl Node and the Decl Kind
            std::cout<<Declaration->getID()<<" "<<Declaration->getDeclKindName()<<"Decl ";
            auto parDecl = currNode.get<Decl>();
            // parent node is either Decl or Stmt
            if (parDecl == 0) {
                auto parStmt = currNode.get<Stmt>();
                std::cout<<currNode.getNodeKind().asStringRef().str()<<" "<<parStmt->getID(*Context);
            }
            else {
                std::cout<<currNode.getNodeKind().asStringRef().str()<<" "<<parDecl->getID();
            }
            std::cout<<"\n";
            
        }

        return true;

    }

    bool VisitStmt(Stmt *Statement) { 

        clang::SourceManager& mgr = Context->getSourceManager();
        if (mgr.isInMainFile(Statement->getBeginLoc())) {

            bool notParent = true;
            ast_type_traits::DynTypedNode currNode = clang::ast_type_traits::DynTypedNode::create(*Statement);
            
            while (notParent) {

                auto parentsList = Context->getParents(currNode);
                for(auto p=parentsList.begin(); p!= parentsList.end(); p++) {
                    //std::cout<<p->getNodeKind().asStringRef().str()<<"\n";
                    auto parDecl = p->get<Decl>();
                    auto parStmt = p->get<Stmt>();
                    if (parDecl!=0) notParent = false;
                    if (parStmt!=0) notParent = false;
                } 

                currNode = *parentsList.begin();

            }

            std::cout<<Statement->getID(*Context)<<" "<<Statement->getStmtClassName()<<" ";
            auto parDecl = currNode.get<Decl>();
            if (parDecl == 0) {
                auto parStmt = currNode.get<Stmt>();
                std::cout<<currNode.getNodeKind().asStringRef().str()<<" "<<parStmt->getID(*Context);
            }
            else {
                std::cout<<currNode.getNodeKind().asStringRef().str()<<" "<<parDecl->getID();
            }
            std::cout<<"\n";

            
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
        std::cout<<Context.getTranslationUnitDecl()->getID()<<" TranslationUnitDecl\n";
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