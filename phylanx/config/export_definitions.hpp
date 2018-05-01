//  Copyright (c) 2007-2012 Hartmut Kaiser
//  Copyright (c)      2011 Bryce Lelbach
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_EXPORT_DEFINITIONS_AUG_25_2017_0653PM)
#define PHYLANX_EXPORT_DEFINITIONS_AUG_25_2017_0653PM

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
# define PHYLANX_SYMBOL_EXPORT      __declspec(dllexport)
# define PHYLANX_SYMBOL_IMPORT      __declspec(dllimport)
# define PHYLANX_SYMBOL_INTERNAL    /* empty */
# define PHYLANX_APISYMBOL_EXPORT   __declspec(dllexport)
# define PHYLANX_APISYMBOL_IMPORT   __declspec(dllimport)
#elif defined(__NVCC__) || defined(__CUDACC__)
# define PHYLANX_SYMBOL_EXPORT      /* empty */
# define PHYLANX_SYMBOL_IMPORT      /* empty */
# define PHYLANX_SYMBOL_INTERNAL    /* empty */
# define PHYLANX_APISYMBOL_EXPORT   /* empty */
# define PHYLANX_APISYMBOL_IMPORT   /* empty */
#elif defined(PHYLANX_HAVE_ELF_HIDDEN_VISIBILITY)
# define PHYLANX_SYMBOL_EXPORT      __attribute__((visibility("default")))
# define PHYLANX_SYMBOL_IMPORT      __attribute__((visibility("default")))
# define PHYLANX_SYMBOL_INTERNAL    __attribute__((visibility("hidden")))
# define PHYLANX_APISYMBOL_EXPORT   __attribute__((visibility("default")))
# define PHYLANX_APISYMBOL_IMPORT   __attribute__((visibility("default")))
#endif

// make sure we have reasonable defaults
#if !defined(PHYLANX_SYMBOL_EXPORT)
# define PHYLANX_SYMBOL_EXPORT      /* empty */
#endif
#if !defined(PHYLANX_SYMBOL_IMPORT)
# define PHYLANX_SYMBOL_IMPORT      /* empty */
#endif
#if !defined(PHYLANX_SYMBOL_INTERNAL)
# define PHYLANX_SYMBOL_INTERNAL    /* empty */
#endif
#if !defined(PHYLANX_APISYMBOL_EXPORT)
# define PHYLANX_APISYMBOL_EXPORT   /* empty */
#endif
#if !defined(PHYLANX_APISYMBOL_IMPORT)
# define PHYLANX_APISYMBOL_IMPORT   /* empty */
#endif

///////////////////////////////////////////////////////////////////////////////
// define the export/import helper macros used by the runtime module
#if defined(PHYLANX_EXPORTS)
# define  PHYLANX_EXPORT             PHYLANX_SYMBOL_EXPORT
# define  PHYLANX_EXCEPTION_EXPORT   PHYLANX_SYMBOL_EXPORT
# define  PHYLANX_API_EXPORT         PHYLANX_APISYMBOL_EXPORT
#else
# define  PHYLANX_EXPORT             PHYLANX_SYMBOL_IMPORT
# define  PHYLANX_EXCEPTION_EXPORT   PHYLANX_SYMBOL_IMPORT
# define  PHYLANX_API_EXPORT         PHYLANX_APISYMBOL_IMPORT
#endif

///////////////////////////////////////////////////////////////////////////////
// define the export/import helper macros to be used for component modules
#if defined(PHYLANX_PLUGIN_EXPORTS)
# define  PHYLANX_PLUGIN_EXPORT     PHYLANX_SYMBOL_EXPORT
#else
# define  PHYLANX_PLUGIN_EXPORT     PHYLANX_SYMBOL_IMPORT
#endif

///////////////////////////////////////////////////////////////////////////////
// define the export/import helper macros to be used for component modules
#if defined(PHYLANX_LIBRARY_EXPORTS)
# define  PHYLANX_LIBRARY_EXPORT    PHYLANX_SYMBOL_EXPORT
#else
# define  PHYLANX_LIBRARY_EXPORT    PHYLANX_SYMBOL_IMPORT
#endif

///////////////////////////////////////////////////////////////////////////////
// helper macro for symbols which have to be exported from the runtime and all
// components
#if defined(PHYLANX_EXPORTS) || defined(PHYLANX_PLUGIN_EXPORTS) || \
    defined(PHYLANX_APPLICATION_EXPORTS) || defined(PHYLANX_LIBRARY_EXPORTS)
# define PHYLANX_ALWAYS_EXPORT      PHYLANX_SYMBOL_EXPORT
#else
# define PHYLANX_ALWAYS_EXPORT      PHYLANX_SYMBOL_IMPORT
#endif

#endif
