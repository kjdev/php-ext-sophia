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

#ifdef HAVE_CONFIG_H
#    include "config.h"
#endif

#include <php.h>
#include <php_ini.h>
#include <Zend/zend_interfaces.h>

#include "php_sophia.h"
#include "cursor.h"
#include "db.h"
#include "exception.h"

zend_class_entry *php_sp_cursor_ce;
static zend_object_handlers php_sp_cursor_handlers;

ZEND_EXTERN_MODULE_GLOBALS(sophia)

ZEND_BEGIN_ARG_INFO_EX(arginfo_sp_cursor___construct, 0, ZEND_RETURN_VALUE, 1)
    ZEND_ARG_INFO(0, db)
    ZEND_ARG_INFO(0, order)
    ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sp_cursor_no_parameters, 0, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

static void
php_sp_cursor_create(zval *obj, zval *db, char *order, int order_len,
                     char *key, int key_len TSRMLS_DC)
{
    php_sp_cursor_t *intern;
    void *database, *object;

    PHP_SP_CURSOR_OBJ(intern, obj, 0);

    intern->db = db;
    zval_add_ref(&intern->db);

    if (order && order_len > 0) {
        smart_str_appendl(&intern->order, order, order_len);
        smart_str_0(&intern->order);
    }

    if (key && key_len > 0) {
        smart_str_appendl(&intern->key, key, key_len);
        smart_str_0(&intern->key);
    }

    intern->sophia.db = php_sp_db_object_get_database(db TSRMLS_CC);
    if (!intern->sophia.db) {
        PHP_SP_EXCEPTION(0, "Can not database object");
        return;
    }

    object = sp_object(intern->sophia.db);

    if (intern->order.c) {
        if (intern->key.c) {
            sp_set(object, "key", intern->key.c, intern->key.len);
        }
        if (sp_set(object, "order", intern->order.c) == -1) {
            smart_str_free(&intern->order);
            PHP_SP_ERR(E_WARNING, "Error iteration order set");
        }
    }

    intern->sophia.cursor = sp_cursor(intern->sophia.db, object);
    if (!intern->sophia.cursor) {
        PHP_SP_EXCEPTION(0, "Can not open cursor");
        return;
    }

    intern->first = 1;
}

PHP_SOPHIA_METHOD(Cursor, __construct)
{
    zval *db;
    char *order = NULL, *key = NULL;
    int order_len = 0, key_len = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O|ss",
                              &db, php_sp_db_ce,
                              &order, &order_len,
                              &key, &key_len) == FAILURE) {
        RETURN_FALSE;
    }

    php_sp_cursor_create(getThis(), db,
                         order, order_len, key, key_len TSRMLS_CC);
}

PHP_SOPHIA_METHOD(Cursor, rewind)
{
    php_sp_cursor_t *intern;
    void *object;

    if (zend_parse_parameters_none() == FAILURE) {
        RETURN_FALSE;
    }

    PHP_SP_CURSOR_OBJ(intern, getThis(), 1);

    if (intern->sophia.current) {
        sp_destroy(intern->sophia.current);
        intern->sophia.current = NULL;
    }

    object = sp_object(intern->sophia.db);

    if (!intern->first && intern->sophia.cursor) {
        sp_destroy(intern->sophia.cursor);

        if (intern->order.c) {
            if (intern->key.c) {
                sp_set(object, "key", intern->key.c, intern->key.len);
            }
            if (sp_set(object, "order", intern->order.c) == -1) {
                PHP_SP_ERR(E_WARNING, "Error iteration order set");
            }
        }

        intern->sophia.cursor = sp_cursor(intern->sophia.db, object);
        if (!intern->sophia.cursor) {
            PHP_SP_EXCEPTION(0, "Can not open cursor");
            RETURN_FALSE;
        }
    }

    intern->sophia.current = sp_get(intern->sophia.cursor, object);
    if (!intern->sophia.current) {
        RETURN_FALSE;
    }

    intern->first = 0;

    RETURN_TRUE;
}

PHP_SOPHIA_METHOD(Cursor, next)
{
    php_sp_cursor_t *intern;
    void *object;

    if (zend_parse_parameters_none() == FAILURE) {
        RETURN_FALSE;
    }

    PHP_SP_CURSOR_OBJ(intern, getThis(), 1);

    if (intern->sophia.current) {
        sp_destroy(intern->sophia.current);
        intern->sophia.current = NULL;
    }

    object = sp_object(intern->sophia.db);
    intern->sophia.current = sp_get(intern->sophia.cursor, object);
}

PHP_SOPHIA_METHOD(Cursor, valid)
{
    php_sp_cursor_t *intern;
    void *object;

    if (zend_parse_parameters_none() == FAILURE) {
        RETURN_FALSE;
    }

    PHP_SP_CURSOR_OBJ(intern, getThis(), 1);

    if (intern->sophia.current) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }
}

PHP_SOPHIA_METHOD(Cursor, current)
{
    php_sp_cursor_t *intern;
    char *value = NULL;
    int value_len = 0;

    if (zend_parse_parameters_none() == FAILURE) {
        RETURN_FALSE;
    }

    PHP_SP_CURSOR_OBJ(intern, getThis(), 1);

    if (!intern->sophia.current) {
        RETURN_FALSE;
    }

    value = sp_get(intern->sophia.current, "value", &value_len);

    if (value_len == 0) {
        RETURN_EMPTY_STRING();
    } else if (value) {
        RETURN_STRINGL(value, value_len, 1);
    } else {
        RETURN_FALSE;
    }
}

PHP_SOPHIA_METHOD(Cursor, key)
{
    php_sp_cursor_t *intern;
    char *key = NULL;
    int key_len = 0;

    if (zend_parse_parameters_none() == FAILURE) {
        RETURN_FALSE;
    }

    PHP_SP_CURSOR_OBJ(intern, getThis(), 1);

    if (!intern->sophia.current) {
        RETURN_FALSE;
    }

    key = sp_get(intern->sophia.current, "key", &key_len);

    if (key_len == 0) {
        RETURN_EMPTY_STRING();
    } else if (key) {
        RETURN_STRINGL(key, key_len, 1);
    } else {
        RETURN_FALSE;
    }
}

PHP_SOPHIA_API void
php_sp_cursor_construct(zval *return_value, zval *db,
                        char *order, int order_len,
                        char *key, int key_len TSRMLS_DC)
{
    object_init_ex(return_value, php_sp_cursor_ce);
    php_sp_cursor_create(return_value, db,
                         order, order_len, key, key_len TSRMLS_CC);
}


static zend_function_entry php_sp_cursor_methods[] = {
    PHP_SOPHIA_ME(Cursor, __construct, arginfo_sp_cursor___construct,
                  ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_SOPHIA_ME(Cursor, rewind,
                  arginfo_sp_cursor_no_parameters, ZEND_ACC_PUBLIC)
    PHP_SOPHIA_ME(Cursor, next,
                  arginfo_sp_cursor_no_parameters, ZEND_ACC_PUBLIC)
    PHP_SOPHIA_ME(Cursor, valid,
                  arginfo_sp_cursor_no_parameters, ZEND_ACC_PUBLIC)
    PHP_SOPHIA_ME(Cursor, current,
                  arginfo_sp_cursor_no_parameters, ZEND_ACC_PUBLIC)
    PHP_SOPHIA_ME(Cursor, key,
                  arginfo_sp_cursor_no_parameters, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

static void
php_sp_cursor_free_storage(void *object TSRMLS_DC)
{
    php_sp_cursor_t *intern = (php_sp_cursor_t *)object;

    if (!intern) {
        return;
    }

    smart_str_free(&intern->order);
    smart_str_free(&intern->key);

    if (intern->sophia.current) {
        sp_destroy(intern->sophia.current);
    }

    if (intern->sophia.cursor) {
        sp_destroy(intern->sophia.cursor);
    }

    if (intern->db) {
        zval_ptr_dtor(&intern->db);
    }

    zend_object_std_dtor(&intern->std TSRMLS_CC);
    efree(object);
}

static zend_object_value
php_sp_cursor_new_ex(zend_class_entry *ce, php_sp_cursor_t **ptr TSRMLS_DC)
{
    php_sp_cursor_t *intern;
    zend_object_value retval;

    intern = (php_sp_cursor_t *)emalloc(sizeof(php_sp_cursor_t));
    memset(intern, 0, sizeof(php_sp_cursor_t));
    if (ptr) {
        *ptr = intern;
    }

    zend_object_std_init(&intern->std, ce TSRMLS_CC);
    object_properties_init(&intern->std, ce);

    retval.handle = zend_objects_store_put(
        intern, (zend_objects_store_dtor_t)zend_objects_destroy_object,
        (zend_objects_free_object_storage_t)php_sp_cursor_free_storage,
        NULL TSRMLS_CC);
    retval.handlers = &php_sp_cursor_handlers;

    return retval;
}

static zend_object_value
php_sp_cursor_new(zend_class_entry *ce TSRMLS_DC)
{
    return php_sp_cursor_new_ex(ce, NULL TSRMLS_CC);
}

PHP_SOPHIA_API int
php_sp_cursor_class_register(TSRMLS_D)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, ZEND_NS_NAME(PHP_SOPHIA_NS, "Cursor"),
                     php_sp_cursor_methods);

    ce.create_object = php_sp_cursor_new;

    php_sp_cursor_ce = zend_register_internal_class(&ce TSRMLS_CC);
    if (php_sp_cursor_ce == NULL) {
        return FAILURE;
    }
    zend_class_implements(php_sp_cursor_ce TSRMLS_CC, 1, zend_ce_iterator);

    memcpy(&php_sp_cursor_handlers, zend_get_std_object_handlers(),
           sizeof(zend_object_handlers));

    php_sp_cursor_handlers.clone_obj = NULL;

    return SUCCESS;
}
