/*
 * Copyright (c) 2015 kjdev
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the 'Software'),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef PHP_SOPHIA_H
#define PHP_SOPHIA_H

#define PHP_SOPHIA_EXT_VERSION "0.0.0"

#define PHP_SOPHIA_NS "Sophia"
#define PHP_SOPHIA_METHOD(classname, name) \
    ZEND_METHOD(Sophia_##classname, name)
#define PHP_SOPHIA_ME(classname, name, arg_info, flags) \
    ZEND_ME(Sophia_##classname, name, arg_info, flags)
#define PHP_SOPHIA_MALIAS(classname, name, alias, arg_info, flags) \
    ZEND_MALIAS(Sophia_##classname, name, alias, arg_info, flags)
#define PHP_SOPHIA_LONG_CONSTANT(name, val) \
    REGISTER_NS_LONG_CONSTANT(PHP_SP_NS, name, val, CONST_CS|CONST_PERSISTENT)
#define PHP_SOPHIA_STRING_CONSTANT(name, val) \
    REGISTER_NS_STRING_CONSTANT(PHP_SP_NS, name, val, CONST_CS|CONST_PERSISTENT)

#define PHP_SOPHIA_DEFAULT_PATH "./storage"

extern zend_module_entry sophia_module_entry;
#define phpext_sophia_ptr &sophia_module_entry

#ifdef PHP_WIN32
#   define PHP_SOPHIA_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#   define PHP_SOPHIA_API __attribute__ ((visibility("default")))
#else
#   define PHP_SOPHIA_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

ZEND_BEGIN_MODULE_GLOBALS(sophia)
    char *path;
ZEND_END_MODULE_GLOBALS(sophia)

#ifdef ZTS
#define PHP_SOPHIA_G(v) TSRMG(sophia_globals_id, zend_sophia_globals *, v)
#else
#define PHP_SOPHIA_G(v) (sophia_globals.v)
#endif

#endif  /* PHP_SOPHIA_H */
