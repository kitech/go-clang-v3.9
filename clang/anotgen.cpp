
#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"

#include "clang/Frontend/ASTUnit.h"
#include "clang/Basic/CodeGenOptions.h"
#include "clang/Frontend/CompilerInstance.h"

///
#include "clang/CodeGen/ModuleBuilder.h"
#include "clang/CodeGen/CGFunctionInfo.h"
#include "clang/CodeGen/CodeGenABITypes.h"

///
#include "llvm/IR/Module.h"

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
    bool clang_Decl_hasSimpleDestructor(void* declx) {
        clang::CXXRecordDecl* decl = (clang::CXXRecordDecl*)declx;
        return decl->hasSimpleDestructor();
    }
    bool clang_Decl_hasUserDeclaredDestructor(void* declx) {
        clang::CXXRecordDecl* decl = (clang::CXXRecordDecl*)declx;
        return decl->hasUserDeclaredDestructor();
    }


    // Type
    bool clang_Type_isFunctionPointerType(void* typex) {
        clang::Type* type_ = (clang::Type*)typex;
        return type_->isFunctionPointerType();
    }
    bool clang_Type_isDependentType(void* typex) {
        clang::Type* type_ = (clang::Type*)typex;
        return type_->isDependentType();
    }
    bool clang_Type_isInstantiationDependentType(void* typex) {
        clang::Type* type_ = (clang::Type*)typex;
        return type_->isInstantiationDependentType();
    }
    bool clang_Type_isTemplateTypeParmType(void* typex) {
        clang::Type* type_ = (clang::Type*)typex;
        return type_->isTemplateTypeParmType();
    }
    bool clang_Type_isUndeducedType(void* typex) {
        clang::Type* type_ = (clang::Type*)typex;
        return type_->isUndeducedType();
    }
    bool clang_Type_isTrivialType(void* typex, clang::ASTUnit* au) {
        clang::Type* type_ = (clang::Type*)typex;
        clang::QualType quty = clang::QualType::getFromOpaquePtr(typex);
        return quty.isTrivialType(au->getASTContext());
    }
    bool clang_Type_isTriviallyCopyableType(void* typex, clang::ASTUnit* au) {
        clang::Type* type_ = (clang::Type*)typex;
        clang::QualType quty = clang::QualType::getFromOpaquePtr(typex);
        return quty.isTriviallyCopyableType(au->getASTContext());
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
    void* clang_Type_getAsCXXRecordDecl(void* typex) {
        clang::Type* type_ = (clang::Type*)typex;
        return type_->getAsCXXRecordDecl();
    }

    // CodeGen/CompilerInstance/Frontend
    void* clang_CreateLLVMCodeGen2(clang::ASTUnit* au) {
        llvm::StringRef ModuleName = "hhh";
        clang::CodeGenOptions *CGO = new clang::CodeGenOptions();
        llvm::LLVMContext *C = new llvm::LLVMContext(); // wow
        clang::CoverageSourceInfo *CoverageInfo = nullptr;

        clang::CodeGenerator *cg;
        cg = clang::CreateLLVMCodeGen(au->getDiagnostics(),
                                      ModuleName,
                                      au->getHeaderSearchOpts(),
                                      au->getPreprocessorOpts(),
                                      *CGO,
                                      *C,
                                      CoverageInfo);
        cg->Initialize(au->getASTContext()); //wow
        return cg;
    }

    const void* clang_CodeGen_arrangeCXXMethodType(clang::CodeGenerator* cg,
                                             const clang::CXXRecordDecl *RD,
                                             const clang::FunctionProtoType *FTP,
                                             const clang::CXXMethodDecl *MD) {
        clang::CodeGen::CodeGenModule &CGM = cg->CGM();
        //MD->getMethodQualifiers().getAddressSpace();
        auto& fni = clang::CodeGen::arrangeCXXMethodType(CGM, RD, FTP, MD);
        int aikind = 0;
        bool issret = false;
        bool insret = false;
        if (!fni.isNoReturn()) {
            const clang::CodeGen::ABIArgInfo &abiai = fni.getReturnInfo();
            aikind = abiai.getKind();
            issret = abiai.isIndirect() && abiai.isSRetAfterThis();
            insret = abiai.isInAlloca() && abiai.isInAlloca();
        }
        printf("incpp getfni %d aikind=%d %d %d\n", fni.usesInAlloca(), aikind, issret, insret);
        // no copy ctor
        // clang::CodeGen::CGFunctionInfo *fnip = new clang::CodeGen::CGFunctionInfo(fni);
        const clang::CodeGen::CGFunctionInfo *fnip = &fni;
        return fnip;
    }

    void* clang_CodeGen_convertFreeFunctionType(clang::CodeGenerator* cg,
                                                const clang::FunctionDecl *FD) {
        llvm::FunctionType* fnty = clang::CodeGen::convertFreeFunctionType(cg->CGM(), FD);
        int argc = fnty->getNumParams();
        printf("incpp fnty %d\n", argc);
        return fnty;
    }

    bool clang_CodeGen_isStructRet(const clang::CodeGen::CGFunctionInfo* fni) {
        if (fni->isNoReturn()) { return false; } // seems not work
        const clang::CodeGen::ABIArgInfo &abiai = fni->getReturnInfo();
        if (abiai.getKind() > 6) { abort(); }
        printf("incpp usesinalloca %d aikind %d\n", fni->usesInAlloca(), abiai.getKind());
        return ((abiai.isInAlloca()) && abiai.getInAllocaSRet()) ||
            ( (abiai.isIndirect()) && abiai.isSRetAfterThis());
    }
    int clang_CodeGen_arg_size(const clang::CodeGen::CGFunctionInfo* fni) {
        return fni->arg_size();
    }

    int llvm_Type_getFunctionNumParams(llvm::Type* typex) {
        return typex->getFunctionNumParams();
    }

    void llvm_Type_delete(llvm::FunctionType* typex) {
        //delete typex;
    }

}; // end extern C

