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

#ifndef PHP_SOPHIA_EXCEPTION_H
#define PHP_SOPHIA_EXCEPTION_H

#include "zend_exceptions.h"

#include "php_sophia.h"

extern PHP_SOPHIA_API zend_class_entry *php_sp_exception_ce;

PHP_SOPHIA_API int php_sp_exception_class_register(TSRMLS_D);

#define PHP_SP_ERR(flag, ...) \
    php_error_docref(NULL TSRMLS_CC, flag, __VA_ARGS__)

#define PHP_SP_EXCEPTION(code, ...) \
    zend_throw_exception_ex(php_sp_exception_ce, code TSRMLS_CC, __VA_ARGS__)

#endif
