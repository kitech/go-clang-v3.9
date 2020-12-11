package clang

import (
	"unsafe"
)

// 尝试扩展一下 go-clang/cindex中的可用函数
//

/*
// #cgo CFLAGS: -std=c99
#cgo CFLAGS: -O0
#cgo LDFLAGS: -lclang -lclang-cpp -lLLVM

#include <stdbool.h>
#include <./clang-c/Index.h>

extern unsigned clang_CXXRecord_isAbstract(CXCursor C);

extern bool clang_DeclIsDeleted(void* declx);
extern bool clang_DeclIsOverloadedOperator(void* declx);
extern void* clang_DeclGetFunctionProtoType(void* declx);
extern bool clang_DeclHasDefaultArg(void* declx) ;
extern void* clang_DeclGetDefaultArg(void* declx);
extern bool clang_Decl_hasSimpleDestructor(void* declx) ;
extern bool clang_Decl_hasUserDeclaredDestructor(void* declx);

extern bool clang_Type_isFunctionPointerType(void* typex);
extern bool clang_Type_isDependentType(void* typex);
extern bool clang_Type_isInstantiationDependentType(void* typex);
extern bool clang_Type_isTemplateTypeParmType(void* typex);
extern bool clang_Type_isUndeducedType(void* typex);
extern bool clang_Type_isTrivialType(void* typex, void*au);
extern bool clang_Type_isTriviallyCopyableType(void* typex, void*au);
extern void* clang_Type_getUnqualifiedDesugaredType(void* typex);
extern void* clang_Type_removeLocalConst(void* typex);
extern void* clang_Type_getAsCXXRecordDecl(void* typex);

extern void* clang_CreateLLVMCodeGen2(void*);
extern void* clang_CodeGen_arrangeCXXMethodType(void* cg,
                                             void *RD,
                                             void *FTP,
                                             void *MD);
extern void* clang_CodeGen_convertFreeFunctionType(void* cg, void *FD);

extern bool clang_CodeGen_isStructRet(void* fni) ;
extern int clang_CodeGen_arg_size(void* fni);

extern int llvm_Type_getFunctionNumParams(void* typex);
extern void llvm_Type_delete(void* typex);
*/
import "C"

// AST/Decl.h:2257 isDeleted()
// AST/Decl.h:1698 hasDefaultArg()

// emulate here

type dummyCXTranslationUnit struct {
	// clang::CIndexer *CIdx;
	CIdx unsafe.Pointer
	// clang::ASTUnit *TheASTUnit;
	TheASTUnit unsafe.Pointer
	// clang::cxstring::CXStringPool *StringPool;
	StringPool unsafe.Pointer
	// void *Diagnostics;
	Dignostics unsafe.Pointer
	// void *OverridenCursorsPool;
	OverridenCursorsPool unsafe.Pointer
	// clang::index::CommentToXMLConverter *CommentToXML;
	CommentToXML unsafe.Pointer
	// unsigned ParsingOptions;
	ParseingOptions C.uint
	// std::vector<std::string> Arguments;
}

func (this TranslationUnit) ASTUnit() unsafe.Pointer {
	tup := unsafe.Pointer(this.c)
	cxtu := (*dummyCXTranslationUnit)(tup)
	return cxtu.TheASTUnit
}

// for decl cursor, data[3] = {Parent, Attr, TU}
// must same as Index.h CXCursor
type dummyCXCursor struct {
	// enum CXCursorKind kind;
	kind C.int
	// int xdata;
	xdata C.int
	// const void *data[3];
	data0 unsafe.Pointer
	data1 unsafe.Pointer
	data2 unsafe.Pointer
}

func cbool2go(v C.bool) bool { return bool(v) }

func (c Cursor) decl() unsafe.Pointer {
	// TODO check if Decl
	return unsafe.Pointer(uintptr(c.c.data[0]))
}
func (this Cursor) ASTUnit() unsafe.Pointer {
	return unsafe.Pointer(uintptr(this.c.data[2]))
}

// => IsFunctionDeleted?
func (c Cursor) IsDeleted() bool {
	rv := C.clang_DeclIsDeleted(c.decl())
	return cbool2go(rv)
}

func (c Cursor) IsOverloadedOperator() bool {
	rv := C.clang_DeclIsOverloadedOperator(c.decl())
	return cbool2go(rv)
}

func (c Cursor) GetFunctionProtoType() unsafe.Pointer {
	rv := C.clang_DeclGetFunctionProtoType(c.decl())
	return rv
}

func (c Cursor) CXXRecord_IsAbstract() bool {
	rv := C.clang_CXXRecord_isAbstract(c.c)
	return rv != 0
}

func (c Cursor) HasDefaultArg() bool {
	rv := C.clang_DeclHasDefaultArg(c.decl())
	return cbool2go(rv)
}

func (c Cursor) GetDefaultArg() unsafe.Pointer {
	rv := C.clang_DeclGetDefaultArg(c.decl())
	return rv
}

func (c Cursor) HasSimpleDestructor() bool {
	rv := C.clang_Decl_hasSimpleDestructor(c.decl())
	return cbool2go(rv)
}

func (c Cursor) HasUserDeclaredDestructor() bool {
	rv := C.clang_Decl_hasUserDeclaredDestructor(c.decl())
	return cbool2go(rv)
}

// fni clang::FunctionProtoType*
func (c Cursor) IsStructRet1(fni unsafe.Pointer) bool {
	rv := C.clang_CodeGen_isStructRet(fni)
	return cbool2go(rv)
}

// 如果是CXXMethod得到的结果包含this
// fni clang::FunctionProtoType*
func (c Cursor) ArgSize(fni unsafe.Pointer) int {
	rv := C.clang_CodeGen_arg_size(fni)
	return int(rv)
}

// for CXType
// isFunctionType
// isTemplateType

// must same as Index.h CXType
type dummyCXType struct {
	kind  C.int
	data0 unsafe.Pointer // clang::Type ptr
	data1 unsafe.Pointer // TU ptr
}

// clang::Type*, not CXType
func (this Type) unpack() unsafe.Pointer {
	return unsafe.Pointer(uintptr(this.c.data[0]))
}
func (this Type) ASTUnit() unsafe.Pointer {
	return unsafe.Pointer(uintptr(this.c.data[1]))
}

func (t Type) IsFunctionPointerType() bool {
	if t.Kind() == Type_Char_S || t.Kind() == Type_Record ||
		t.Kind() == Type_Elaborated || t.Kind() == Type_Unexposed {
		// why crash?
		return false
	}
	rv := C.clang_Type_isFunctionPointerType(t.unpack())
	return cbool2go(rv)
}
func (t Type) IsDependentType() bool {
	rv := C.clang_Type_isDependentType(t.unpack())
	return cbool2go(rv)
}
func (t Type) IsInstantiationDependentType() bool {
	rv := C.clang_Type_isInstantiationDependentType(t.unpack())
	return cbool2go(rv)
}
func (t Type) IsTemplateTypeParmType() bool {
	rv := C.clang_Type_isTemplateTypeParmType(t.unpack())
	return cbool2go(rv)
}
func (t Type) IsUndeducedType() bool {
	rv := C.clang_Type_isUndeducedType(t.unpack())
	return cbool2go(rv)
}
func (t Type) IsTrivialType() bool {
	rv := C.clang_Type_isTrivialType(t.unpack(), t.ASTUnit())
	return cbool2go(rv)
}
func (t Type) IsTriviallyCopyableType() bool {
	rv := C.clang_Type_isTriviallyCopyableType(t.unpack(), t.ASTUnit())
	return cbool2go(rv)
}

// wow, clang::Type is immutable
// 似乎不管用
func (t Type) UnqualifiedDesugaredType() Type {
	p := C.clang_Type_getUnqualifiedDesugaredType(t.unpack())
	t2 := t
	t2.c.data[0] = C.ulong(uintptr(p))
	return t2
}

// 似乎不管用
func (t Type) RemoveLocalConst() Type {
	p := C.clang_Type_removeLocalConst(t.unpack())
	t2 := t
	t2.c.data[0] = C.ulong(uintptr(p))
	return t2
}

// CompilerInstance/CodeGen
type CodeGenerator struct {
	Cthis unsafe.Pointer
}

func NewCodeGenerator2(tu TranslationUnit) CodeGenerator {
	this := CodeGenerator{}
	this.Cthis = C.clang_CreateLLVMCodeGen2(tu.ASTUnit())
	return this
}

type CGFunctionInfo struct {
	Cthis unsafe.Pointer
}

// 有模板返回值/参数时会crash，其他情况可用了
// param cursor CXXMethodDecl/FunctionDecl
// param parent ClassDecl
//
func (this *CodeGenerator) ArrangeCXXMethodType(cursor, parent Cursor) unsafe.Pointer {
	fnproto := cursor.GetFunctionProtoType()
	recdecl := parent.decl()
	mthdecl := cursor.decl()
	if cursor.CXXMethod_IsStatic() {
		// recdecl = nil
	}
	rv := C.clang_CodeGen_arrangeCXXMethodType(this.Cthis, recdecl, fnproto, mthdecl)
	return rv
}

// cursor FunctionDecl*
// return llvm::FunctionType*, need delete
func (this *CodeGenerator) ConvertFreeFunctionType(cursor Cursor) unsafe.Pointer {
	rv := C.clang_CodeGen_convertFreeFunctionType(this.Cthis, cursor.decl())
	return rv
}

// 得到的结果包含sret，但是不包含this
// cursor FunctionDecl/CXXMethodDecl...
func (this *CodeGenerator) LLVMNumParams(cursor Cursor) int {
	fnty := this.ConvertFreeFunctionType(cursor)
	rv := C.llvm_Type_getFunctionNumParams(fnty)
	C.llvm_Type_delete(fnty)
	return int(rv)
}

//////////
func assertok(cond bool, args ...interface{}) {
	if !cond {
		args := append([]interface{}{"assert failed"}, args...)
		panic(args)
	}
}
