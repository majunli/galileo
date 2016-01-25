#ifndef __GALILEO_BASE_CONFIG_H__
#define __GALILEO_BASE_CONFIG_H__

#if !defined(_MSC_VER) || _MSC_VER < 1800
#error "本项目只支持 MSVS-2013"
#endif

#undef DEBUG
#ifndef NDEBUG
#define DEBUG
#endif // !NDEBUG


#endif // !__GALILEO_BASE_CONFIG_H__
