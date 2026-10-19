// A minimal IL2CPP metadata provider for exercising the real resolver.
#include <cstdint>
#include <cstddef>
struct Type { int kind; const char *name; bool byref; bool enumeration; };
static Type integer{8,"Int32",false,false};
static Type resultType{1,"Void",false,false};
static Type armType{17,"CompositeArmData",false,false};
static int armSize=12;
extern "C" {
void *il2cpp_field_get_type(const void *) {return &armType;}
std::int32_t il2cpp_class_value_size(const void *,std::uint32_t *alignment) {if(alignment)*alignment=4;return armSize;}
std::size_t il2cpp_field_get_offset(const void *) {return 8;}
void fixture_arm_size(int size) {armSize=size;}
void *il2cpp_method_get_param(const void *, unsigned) { return &integer; }
void *il2cpp_method_get_return_type(const void *) { return &resultType; }
bool il2cpp_method_is_instance(const void *) { return true; }
int il2cpp_type_get_type(const void *p) { return static_cast<const Type *>(p)->kind; }
bool il2cpp_type_is_byref(const void *p) { return static_cast<const Type *>(p)->byref; }
void *il2cpp_class_from_type(const void *p) { return const_cast<void *>(p); }
const char *il2cpp_class_get_name(const void *p) { return static_cast<const Type *>(p)->name; }
bool il2cpp_class_is_enum(const void *p) { return static_cast<const Type *>(p)->enumeration; }
void fixture_result(int kind,const char *name) {resultType={kind,name,false,false};}
void fixture_type(int kind, const char *name, bool byref) { integer={kind,name,byref,false}; }
}
