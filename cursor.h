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

#ifndef PHP_SOPHIA_CURSOR_H
#define PHP_SOPHIA_CURSOR_H

#ifdef ZEND_ENGINE_3
#include <ext/standard/php_smart_string.h>
#else
#include <ext/standard/php_smart_str.h>
#endif

#include "sophia.h"

#include "php_sophia.h"
#include "db.h"
#include "exception.h"

typedef struct {
    void *env;
    void *db;
    void *cursor;
    void *current;
} php_sp_object_t;

typedef struct {
#ifndef ZEND_ENGINE_3
    zend_object std;
#endif
#ifdef ZEND_ENGINE_3
    zval db;
    smart_string order;
    zval key;
#else
    zval *db;
    smart_str order;
    zval *key;
#endif
    php_sp_object_t sophia;
#ifdef ZEND_ENGINE_3
    zend_object std;
#endif
} php_sp_cursor_t;

extern PHP_SOPHIA_API zend_class_entry *php_sp_cursor_ce;

PHP_SOPHIA_API int php_sp_cursor_class_register(TSRMLS_D);
PHP_SOPHIA_API void php_sp_cursor_construct(zval *return_value, zval *db, char *order, int order_len, zval *key TSRMLS_DC);


static inline php_sp_cursor_t * php_sp_cursor_object_fetch(zval *zv, int check)
{
    php_sp_cursor_t *self;
#ifdef ZEND_ENGINE_3
    self = (php_sp_cursor_t *)((char *)Z_OBJ_P(zv) - XtOffsetOf(php_sp_cursor_t, std));
    if ((check) && (!(self) || !php_sp_db_object_get_object(&(self)->db TSRMLS_CC)))
#else
    TSRMLS_FETCH();
    self = (php_sp_cursor_t *)zend_object_store_get_object(zv TSRMLS_CC);
    if ((check) && (!(self) || !php_sp_db_object_get_object((self)->db TSRMLS_CC)))
#endif
    {
        PHP_SP_EXCEPTION(0, "Can not iterate on closed database");
        return NULL;
    }
    return self;
}

#define PHP_SP_CURSOR_OBJ_NOCHECK(zv) php_sp_cursor_object_fetch(zv, 0)
#define PHP_SP_CURSOR_OBJ(zv) php_sp_cursor_object_fetch(zv, 1)

#endif
