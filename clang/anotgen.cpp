
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

using namespace clang;
using namespace CodeGen;


static CXTypeKind GetBuiltinTypeKind(const BuiltinType *BT) {
#define BTCASE(K) case BuiltinType::K: return CXType_##K
  switch (BT->getKind()) {
    BTCASE(Void);
    BTCASE(Bool);
    BTCASE(Char_U);
    BTCASE(UChar);
    BTCASE(Char16);
    BTCASE(Char32);
    BTCASE(UShort);
    BTCASE(UInt);
    BTCASE(ULong);
    BTCASE(ULongLong);
    BTCASE(UInt128);
    BTCASE(Char_S);
    BTCASE(SChar);
    case BuiltinType::WChar_S: return CXType_WChar;
    case BuiltinType::WChar_U: return CXType_WChar;
    BTCASE(Short);
    BTCASE(Int);
    BTCASE(Long);
    BTCASE(LongLong);
    BTCASE(Int128);
    //  BTCASE(Half);
    BTCASE(Float);
    BTCASE(Double);
    BTCASE(LongDouble);
    // BTCASE(ShortAccum);
    // BTCASE(Accum);
    // BTCASE(LongAccum);
    // BTCASE(UShortAccum);
    // BTCASE(UAccum);
    // BTCASE(ULongAccum);
    //BTCASE(Float16);
    BTCASE(Float128);
    BTCASE(NullPtr);
    BTCASE(Overload);
    BTCASE(Dependent);
    BTCASE(ObjCId);
    BTCASE(ObjCClass);
    BTCASE(ObjCSel);
    //#define IMAGE_TYPE(ImgType, Id, SingletonId, Access, Suffix) BTCASE(Id);
    //#include "clang/Basic/OpenCLImageTypes.def"
    //#undef IMAGE_TYPE
    //#define EXT_OPAQUE_TYPE(ExtType, Id, Ext) BTCASE(Id);
    //#include "clang/Basic/OpenCLExtensionTypes.def"
    // BTCASE(OCLSampler);
    // BTCASE(OCLEvent);
    // BTCASE(OCLQueue);
    // BTCASE(OCLReserveID);
  default:
    return CXType_Unexposed;
  }
#undef BTCASE
}
static CXTypeKind GetTypeKind(QualType T) {
  const Type *TP = T.getTypePtrOrNull();
  if (!TP)
    return CXType_Invalid;
#define TKCASE(K) case Type::K: return CXType_##K
  switch (TP->getTypeClass()) {
    case Type::Builtin:
      return GetBuiltinTypeKind(cast<BuiltinType>(TP));
    TKCASE(Complex);
    TKCASE(Pointer);
    TKCASE(BlockPointer);
    TKCASE(LValueReference);
    TKCASE(RValueReference);
    TKCASE(Record);
    TKCASE(Enum);
    TKCASE(Typedef);
    TKCASE(ObjCInterface);
    //TKCASE(ObjCObject);
    TKCASE(ObjCObjectPointer);
    // TKCASE(ObjCTypeParam);
    TKCASE(FunctionNoProto);
    TKCASE(FunctionProto);
    TKCASE(ConstantArray);
    TKCASE(IncompleteArray);
    TKCASE(VariableArray);
    TKCASE(DependentSizedArray);
    TKCASE(Vector);
    // TKCASE(ExtVector);
    TKCASE(MemberPointer);
    TKCASE(Auto);
    TKCASE(Elaborated);
    // TKCASE(Pipe);
    // TKCASE(Attributed);
    // TKCASE(Atomic);
    default:
      return CXType_Unexposed;
  }
#undef TKCASE
}

extern "C" {
    // Decl
    bool clang_DeclIsDeleted(void* declx) {
        FunctionDecl* decl = (FunctionDecl*)declx;
        return decl->isDeleted();
    }
    bool clang_DeclIsOverloadedOperator(void* declx) {
        FunctionDecl* decl = (FunctionDecl*)declx;
        return decl->isOverloadedOperator();
    }

    void* clang_DeclGetFunctionProtoType(void* declx) {
        FunctionDecl* decl = (FunctionDecl*)declx;
        auto *TSI = decl->getTypeSourceInfo();
        QualType T = TSI ? TSI->getType() : decl->getType();
        const auto *FPT = T->getAs<FunctionProtoType>();
        return (void*)FPT;
    }

    bool clang_DeclHasDefaultArg(void* declx) {
        ParmVarDecl* decl = (ParmVarDecl*)declx;
        return decl->hasDefaultArg();
    }

    void* clang_DeclGetDefaultArg(void* declx) {
        ParmVarDecl* decl = (ParmVarDecl*)declx;
        return decl->getDefaultArg();
    }
    bool clang_Decl_hasSimpleDestructor(void* declx) {
        CXXRecordDecl* decl = (CXXRecordDecl*)declx;
        return decl->hasSimpleDestructor();
    }
    bool clang_Decl_hasUserDeclaredDestructor(void* declx) {
        CXXRecordDecl* decl = (CXXRecordDecl*)declx;
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

    int clang_Type_getKind(clang::Type* typex) {
        QualType t = QualType::getFromOpaquePtr(typex);
        return GetTypeKind(t);
    }

    void* clang_Type_removeLocalConst(void* typex) {
        clang::Type* type_ = (clang::Type*)typex;
        QualType quty = QualType::getFromOpaquePtr(typex);
        quty.removeLocalConst();
        quty.removeLocalFastQualifiers();
        quty = quty.getLocalUnqualifiedType();
        quty = quty.getUnqualifiedType();
            //CanQualType canqty = type_->getCanonicalTypeUnqualified();
        return quty.getAsOpaquePtr();
    }
    void* clang_Type_getAsCXXRecordDecl(void* typex) {
        clang::Type* type_ = (clang::Type*)typex;
        return type_->getAsCXXRecordDecl();
    }

    // CodeGen/CompilerInstance/Frontend
    void* clang_CreateLLVMCodeGen2(ASTUnit* au) {
        llvm::StringRef ModuleName = "hhh";
        CodeGenOptions *CGO = new CodeGenOptions();
        llvm::LLVMContext *C = new llvm::LLVMContext(); // wow
        CoverageSourceInfo *CoverageInfo = nullptr;

        CodeGenerator *cg;
        cg = CreateLLVMCodeGen(au->getDiagnostics(),
                                      ModuleName,
                                      au->getHeaderSearchOpts(),
                                      au->getPreprocessorOpts(),
                                      *CGO,
                                      *C,
                                      CoverageInfo);
        cg->Initialize(au->getASTContext()); //wow
        return cg;
    }

    const void* clang_CodeGen_arrangeFreeFunctionType(CodeGenerator* cg,
                                                      FunctionProtoType *Ty) {
        CodeGenModule &CGM = cg->CGM();
        CanQual<FunctionProtoType> canty =
            CanQual<FunctionProtoType>::getFromOpaquePtr(Ty);
        auto& fni = arrangeFreeFunctionType(CGM, canty);
        return &fni;
    }

    const void* clang_CodeGen_arrangeCXXMethodType(CodeGenerator* cg,
                                             const CXXRecordDecl *RD,
                                             const FunctionProtoType *FTP,
                                             const CXXMethodDecl *MD) {
        CodeGenModule &CGM = cg->CGM();
        //MD->getMethodQualifiers().getAddressSpace();
        auto& fni = arrangeCXXMethodType(CGM, RD, FTP, MD);
        int aikind = 0;
        bool issret = false;
        bool insret = false;
        if (!fni.isNoReturn()) {
            const ABIArgInfo &abiai = fni.getReturnInfo();
            aikind = abiai.getKind();
            issret = abiai.isIndirect() && abiai.isSRetAfterThis();
            insret = abiai.isInAlloca() && abiai.isInAlloca();
        }
        printf("incpp getfni %d aikind=%d %d %d\n", fni.usesInAlloca(), aikind, issret, insret);
        // no copy ctor
        // clang::CodeGen::CGFunctionInfo *fnip = new clang::CodeGen::CGFunctionInfo(fni);
        const CGFunctionInfo *fnip = &fni;
        return fnip;
    }

    void* clang_CodeGen_convertFreeFunctionType(CodeGenerator* cg,
                                                const FunctionDecl *FD) {
        llvm::FunctionType* fnty = convertFreeFunctionType(cg->CGM(), FD);
        int argc = fnty->getNumParams();
        return fnty;
    }

    void* clang_CodeGen_convertTypeForMemory(CodeGenerator* cg,
                                             clang::Type* type_) {
        QualType quty = QualType::getFromOpaquePtr(type_);
        llvm::Type* anty = convertTypeForMemory(cg->CGM(), quty);
        return anty;
    }

    bool clang_CodeGen_isStructRet(const CGFunctionInfo* fni) {
        // if (fni->isNoReturn()) { return false; } // seems not work
        const ABIArgInfo &abiai = fni->getReturnInfo();
        // if (abiai.getKind() > 6) { abort(); }
        printf("incpp usesinalloca %d aikind %d\n", fni->usesInAlloca(), abiai.getKind());
        return abiai.isInAlloca() || abiai.isIndirect();
        // return ((abiai.isInAlloca()) && abiai.getInAllocaSRet()) ||
        // ( (abiai.isIndirect()) && abiai.isSRetAfterThis());
    }
    int clang_CodeGen_arg_size(const CGFunctionInfo* fni) {
        return fni->arg_size();
    }
    void* clang_CodeGen_getArgStruct(const CGFunctionInfo* fni) {
        return fni->getArgStruct();
    }
    int clang_CodeGen_ABIArgInfoKind(const CGFunctionInfo* fni, int idx) {
        assert(idx>=-1 && idx <= 32);
        if (idx == -1) {
            const ABIArgInfo &abiai = fni->getReturnInfo();
            return abiai.getKind();
        }else{
            auto args = fni->arguments();
            return args[idx].info.getKind();
        }
    }
    const void* clang_CodeGen_ABIArgInfoType(const CGFunctionInfo* fni, int idx) {
        assert(idx>=-1 && idx <= 32);
        if (idx == -1) {
            CanQualType ty = fni->getReturnType();
            return ty.getTypePtr();
        }else{
            auto args = fni->arguments();
            return args[idx].type.getTypePtr();
        }
    }

    int llvm_Type_getFunctionNumParams(llvm::Type* typex) {
        return typex->getFunctionNumParams();
    }

    int llvm_Type_getStructNumElements(llvm::StructType* typex) {
        return typex->getNumElements();
    }

    void llvm_Type_delete(llvm::FunctionType* typex) {
        //delete typex;
    }

}; // end extern C

