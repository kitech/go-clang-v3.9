package clang

import (
	"unsafe"
)

// 尝试扩展一下 go-clang/cindex中的可用函数
//

/*
// #cgo CFLAGS: -std=c99
#cgo LDFLAGS: -lclang -lclang-cpp -lLLVM

#include <stdbool.h>
#include <./clang-c/Index.h>

extern unsigned clang_CXXRecord_isAbstract(CXCursor C);

extern bool clang_DeclIsDeleted(void* declx);
extern bool clang_DeclIsOverloadedOperator(void* declx);
extern void* clang_DeclGetFunctionProtoType(void* declx);
extern bool clang_DeclHasDefaultArg(void* declx) ;
extern void* clang_DeclGetDefaultArg(void* declx);

extern bool clang_Type_isFunctionPointerType(void* typex);
extern void* clang_Type_getUnqualifiedDesugaredType(void* typex);
extern void* clang_Type_removeLocalConst(void* typex);

extern void* clang_CreateLLVMCodeGen();
*/
import "C"

// AST/Decl.h:2257 isDeleted()
// AST/Decl.h:1698 hasDefaultArg()

// emulate here

type dummyCXTranslationUnit struct {
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

func (t Type) IsFunctionPointerType() bool {
	if t.Kind() == Type_Char_S || t.Kind() == Type_Record ||
		t.Kind() == Type_Elaborated || t.Kind() == Type_Unexposed {
		// why crash?
		return false
	}
	rv := C.clang_Type_isFunctionPointerType(t.unpack())
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

func NewCodeGenerator() CodeGenerator {
	this := CodeGenerator{}
	this.Cthis = C.clang_CreateLLVMCodeGen()
	return this
}
