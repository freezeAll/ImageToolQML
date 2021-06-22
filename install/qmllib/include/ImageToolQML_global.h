//
// Created by lyric on 2021/4/22.
//
#pragma once
#ifndef BUILD_STATIC
# if defined(IMAGETOOLQML_LIB)
#  define IMAGETOOLQML_EXPORT Q_DECL_EXPORT
# else
#  define IMAGETOOLQML_EXPORT Q_DECL_IMPORT
# endif
#else
# define IMAGETOOLQML_EXPORT
#endif