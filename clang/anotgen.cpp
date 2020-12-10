
#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"

#include "/usr/include/clang/Basic/CodeGenOptions.h"
#include "clang/Frontend/CompilerInstance.h"

///
#include "clang/CodeGen/ModuleBuilder.h"
#include "clang/CodeGen/CGFunctionInfo.h"
#include "clang/CodeGen/CodeGenABITypes.h"

extern "C" {
    // Decl
    bool clang_DeclIsDeleted(void* declx) {
        clang::FunctionDecl* decl = (clang::FunctionDecl*)declx;
        return decl->isDeleted();
    }
    bool clang_DeclIsOverloadedOperator(void* declx) {
        clang::FunctionDecl* decl = (clang::FunctionDecl*)declx;
        return decl->isOverloadedOperator();
    }

    void* clang_DeclGetFunctionProtoType(void* declx) {
        clang::FunctionDecl* decl = (clang::FunctionDecl*)declx;
        auto *TSI = decl->getTypeSourceInfo();
        clang::QualType T = TSI ? TSI->getType() : decl->getType();
        const auto *FPT = T->getAs<clang::FunctionProtoType>();
        return (void*)FPT;
    }

    bool clang_DeclHasDefaultArg(void* declx) {
        clang::ParmVarDecl* decl = (clang::ParmVarDecl*)declx;
        return decl->hasDefaultArg();
    }

    void* clang_DeclGetDefaultArg(void* declx) {
        clang::ParmVarDecl* decl = (clang::ParmVarDecl*)declx;
        return decl->getDefaultArg();
    }


    // Type
    bool clang_Type_isFunctionPointerType(void* typex) {
        clang::Type* type_ = (clang::Type*)typex;
        return type_->isFunctionPointerType();
    }
    void* clang_Type_getUnqualifiedDesugaredType(void* typex) {
        clang::Type* type_ = (clang::Type*)typex;
        return (void*)type_->getUnqualifiedDesugaredType();
    }

    void* clang_Type_removeLocalConst(void* typex) {
        clang::Type* type_ = (clang::Type*)typex;
        clang::QualType quty = clang::QualType::getFromOpaquePtr(typex);
        quty.removeLocalConst();
        quty.removeLocalFastQualifiers();
        quty = quty.getLocalUnqualifiedType();
        quty = quty.getUnqualifiedType();
            //clang::CanQualType canqty = type_->getCanonicalTypeUnqualified();
        return quty.getAsOpaquePtr();
    }

    // CodeGen/CompilerInstance/Frontend
    void* clang_CreateLLVMCodeGen() {
        clang::CompilerInstance ci;

        clang::CodeGenerator *cg;
        clang::DiagnosticsEngine &Diags = ci.getDiagnostics();
        llvm::StringRef ModuleName = "hhh";
        const clang::HeaderSearchOptions &HeaderSearchOpts = ci.getHeaderSearchOpts();
        const clang::PreprocessorOptions &PreprocessorOpts = ci.getPreprocessorOpts();
        clang::CodeGenOptions CGO;
        llvm::LLVMContext C;
        clang::CoverageSourceInfo *CoverageInfo = nullptr;

        cg = clang::CreateLLVMCodeGen(Diags,
                                      ModuleName,
                                      HeaderSearchOpts,
                                      PreprocessorOpts,
                                      CGO,
                                      C,
                                      CoverageInfo);
        return cg;
    }

}; // end extern C

