#ifndef __GALILEO_BASE_BASETYPES_H__
#define __GALILEO_BASE_BASETYPES_H__

#include <cstdint>

#undef DISALLOW_COPY_AND_ASSIGN
// 限制编译器自动生成的类的拷贝构造函数和赋值函数
// 注意：使用时应该声明为私有方法
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

template <typename T, size_t N>
char(&ArraySizeHelper(T(&array)[N]))[N];
// 正确计算数组中的元素数目
#define arraysize(array) (sizeof(ArraySizeHelper(array)))

#endif // !__GALILEO_BASE_BASETYPES_H__
