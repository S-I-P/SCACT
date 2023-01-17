/*
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
        name
        sourcerange
        storage_container static/const/volatile/virtual
        num parameters(1, 2, ...N)
        return type
        param1 type
        param2 type
        ...
        paramN type

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
#include "clang/AST/ASTTypeTraits.h"

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
            //enum
            if(!strcmp(Declaration->getDeclKindName(), "Enum")) {
                EnumDecl *enumDecl = llvm::dyn_cast<EnumDecl>(Declaration);
                if (enumDecl->isCompleteDefinition()) {
                    std::cout<<"<"<<enumDecl->getKindName().str()<<">"<<"\n";
                    std::cout<<enumDecl->getNameAsString()<<"\n";
                    // sourcerange
                    SourceRange srcRange = enumDecl->getSourceRange();
                    if (srcRange.isValid()) {
                        std::cout<<srcRange.printToString(mgr)<<"\n";
                    }
                    else {
                        std::cout<<"invalid SourceRange\n";
                    }
                    //print enum fields
                    std::string fields = "";
                    int totalFields = 0;
                    for(auto enum_iter= enumDecl->enumerator_begin(); enum_iter!=enumDecl->enumerator_end(); enum_iter++) {
                        totalFields += 1;
                        fields = fields + enum_iter->getNameAsString() + "\n";
                        //std::cout<<enum_iter->getNameAsString()<<"\n";
                    }
                    std::cout<<totalFields<<'\n'<<fields;
                }
            }
            //struct, union, class
            else if(!strcmp(Declaration->getDeclKindName(), "Record") || !strcmp(Declaration->getDeclKindName(), "CXXRecord")) {
                RecordDecl *record = llvm::dyn_cast<RecordDecl>(Declaration);
                if (record->isCompleteDefinition()) {
                    std::cout<<"<"<<record->getKindName().str()<<">"<<"\n";
                    std::string recordName = record->getNameAsString();
                    std::cout<<recordName<<"\n";
                    // sourcerange
                    SourceRange srcRange = record->getSourceRange();
                    if (srcRange.isValid()) {
                        std::cout<<srcRange.printToString(mgr)<<"\n";
                    }
                    else {
                        std::cout<<"invalid SourceRange\n";
                    }
                    // fields
                    std::string fields = "";
                    int totalFields = 0;
                    for(auto field_iter= record->field_begin(); field_iter!=record->field_end(); field_iter++) {
                        totalFields += 1;
                        fields = fields + field_iter->getNameAsString() + " " + field_iter->getType().getAsString() + "\n";
                        //std::cout<<field_iter->getNameAsString()<<" "<<field_iter->getType().getAsString()<<"\n";
                    }
                    std::cout<<totalFields<<'\n'<<fields;
                    std::cout<<Declaration->getDeclKindName()<<"\n";
                    //cpp record methods
                    if(!strcmp(Declaration->getDeclKindName(), "CXXRecord")) {
                        //cast to CXXRecordDecl
                        CXXRecordDecl *cxxRecord = llvm::dyn_cast<CXXRecordDecl>(Declaration);
                        //methods
                        std::string methods = "";
                        std::string constructor = "";
                        std::string destructor = "";
                        std::string destStartStr = "~";
                        int totalMethods = 0;
                        for(auto method_iter = cxxRecord->method_begin(); method_iter != cxxRecord->method_end(); method_iter++) {
                            //check method user defined
                            if(method_iter->isUserProvided()) {
                                totalMethods += 1;
                                //method name
                                std::string methodName = method_iter->getNameAsString();
                                std::string method = methodName + "\n";
                                //source range
                                SourceRange methodSrcRange = method_iter->getSourceRange();
                                if (methodSrcRange.isValid()) {
                                    method += methodSrcRange.printToString(mgr) + "\n";
                                }
                                else {
                                    method += "invalid SourceRange\n";
                                }

                                method += std::string(SC[method_iter->getStorageClass()]) +" ";
                                
                                if(method_iter->isStatic()) {
                                    method += "static ";
                                }
                                //if(method_iter->isInstance()) {
                                //    method += "instance ";
                                //}
                                if(method_iter->isConst()) {
                                    method += "const ";
                                }
                                if(method_iter->isVolatile()) {
                                    method += "volatile ";
                                }
                                if(method_iter->isVirtual()) {
                                    method += "virtual ";
                                }
                                //return type
                                method += method_iter->getReturnType().getAsString() +"\n";
                                //parameters
                                int numParams = method_iter->getNumParams();
                                method = method + std::to_string(numParams) + "\n";
                                
                                for(int i=0; i<numParams; i++) {
                                    auto param = method_iter->parameters()[i];
                                    method = method + param->getNameAsString() + " " + param->getOriginalType().getAsString() + "\n";
                                }
                                method += "\n";
                                //constructor or destructor or
                                if(methodName==recordName) {
                                    constructor += method;
                                }
                                else if(methodName == (destStartStr+recordName)) {
                                    destructor += method;
                                }
                                else {
                                    methods += method;
                                }
                            }
                        }
                        std::cout<<totalMethods<<"\n";
                        if(totalMethods) {
                            std::cout<<"<constructor>\n"<<constructor<<"<constructor>\n";
                            std::cout<<"<destructor>\n"<<destructor<<"<destructor>\n";
                            std::cout<<"<method>\n"<<methods<<"<method>\n";
                        }
                    }
                }
            }
            else if(!strcmp(Declaration->getDeclKindName(), "Typedef")) {
                TypedefDecl *Typedef = llvm::dyn_cast<TypedefDecl>(Declaration);
                std::cout<<"<typedef>\n";
                std::cout<<Typedef->getNameAsString()<<"\n"<<Typedef->getUnderlyingType().getAsString()<<"\n";
                SourceRange srcRange = Typedef->getSourceRange();
                if (srcRange.isValid()) {
                    std::cout<<srcRange.printToString(mgr)<<"\n";
                }
                else {
                    std::cout<<"invalid SourceRange\n";
                }
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