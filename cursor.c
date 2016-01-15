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
                     zval *key TSRMLS_DC)
{
    php_sp_cursor_t *intern;
    void *database, *object;

    intern = PHP_SP_CURSOR_OBJ_NOCHECK(obj);
    if (!intern) {
        PHP_SP_EXCEPTION(0, "Can not cursor object");
        return;
    }

#ifdef ZEND_ENGINE_3
    intern->db = *db;
#else
    intern->db = db;
#endif
    zval_add_ref(&intern->db);

    if (order && order_len > 0) {
#ifdef ZEND_ENGINE_3
        smart_string_appendl(&intern->order, order, order_len);
        smart_string_0(&intern->order);
#else
        smart_str_appendl(&intern->order, order, order_len);
        smart_str_0(&intern->order);
#endif
    }

    intern->sophia.db = php_sp_db_object_get_database(db TSRMLS_CC);
    if (!intern->sophia.db) {
        PHP_SP_EXCEPTION(0, "Can not database object");
        return;
    }

    object = sp_object(intern->sophia.db);

    if (intern->order.c) {
        if (sp_set(object, "order", intern->order.c) == -1) {
#ifdef ZEND_ENGINE_3
            smart_string_free(&intern->order);
#else
            smart_str_free(&intern->order);
#endif
            PHP_SP_ERR(E_WARNING, "Error iteration order set");
        }
    }

#ifdef ZEND_ENGINE_3
    array_init(&intern->key);
#else
    MAKE_STD_ZVAL(intern->key);
    array_init(intern->key);
#endif

    if (key) {
        switch (Z_TYPE_P(key)) {
            default:
                convert_to_string(key);
            case IS_STRING:
                sp_set(object, "key", Z_STRVAL_P(key), Z_STRLEN_P(key));
#ifdef ZEND_ENGINE_3
                zend_hash_str_add(Z_ARRVAL(intern->key),
                                  "key", sizeof("key")-1, key);
                zval_add_ref(key);
#else
                zend_hash_add(Z_ARRVAL_P(intern->key), "key", sizeof("key"),
                              &key, sizeof(zval*), NULL);
                zval_add_ref(&key);
#endif
                break;
            case IS_ARRAY: {
#ifdef ZEND_ENGINE_3
                zend_string *str_key;
                ulong index_key;
                zval *index;

                ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(key),
                                          index_key, str_key, index) {
                    switch (Z_TYPE_P(index)) {
                        case IS_STRING:
                            sp_set(object, ZSTR_VAL(str_key),
                                   Z_STRVAL_P(index), Z_STRLEN_P(index));
                            zend_hash_add(Z_ARRVAL(intern->key),
                                          str_key, index);
                            zval_add_ref(index);
                            break;
                        case IS_LONG:
                            sp_set(object, ZSTR_VAL(str_key),
                                   &Z_LVAL_P(index), sizeof(Z_LVAL_P(index)));
                            zend_hash_add(Z_ARRVAL(intern->key),
                                          str_key, index);
                            zval_add_ref(index);
                            break;
                        default:
                            PHP_SP_ERR(E_WARNING, "Invalid key type");
                    }
                } ZEND_HASH_FOREACH_END();
#else
                zval **index;
                uint str_key_len;
                char *str_key;
                ulong index_key;
                uint32_t n;

                for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(key));
                     zend_hash_get_current_data(Z_ARRVAL_P(key),
                                                (void *)&index) == SUCCESS;
                     zend_hash_move_forward(Z_ARRVAL_P(key))) {
                    if (zend_hash_get_current_key_ex(
                            Z_ARRVAL_P(key), &str_key, &str_key_len,
                            &index_key, 0, NULL) == HASH_KEY_IS_STRING) {
                        switch (Z_TYPE_PP(index)) {
                            case IS_STRING:
                                sp_set(object, str_key,
                                       Z_STRVAL_PP(index), Z_STRLEN_PP(index));
                                zend_hash_add(Z_ARRVAL_P(intern->key),
                                              str_key, str_key_len,
                                              index, sizeof(zval*), NULL);
                                zval_add_ref(index);
                                break;
                            case IS_LONG:
                                sp_set(object, str_key,
                                       &Z_LVAL_PP(index),
                                       sizeof(Z_LVAL_PP(index)));
                                zend_hash_add(Z_ARRVAL_P(intern->key),
                                              str_key, str_key_len,
                                              index, sizeof(zval*), NULL);
                                zval_add_ref(index);
                                break;
                            default:
                                PHP_SP_ERR(E_WARNING, "Invalid key type");
                        }
                    }
                }
#endif
                break;
            }
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
    zval *db = NULL;
    char *order = NULL;
#ifdef ZEND_ENGINE_3
    size_t order_len = 0;
#else
    int order_len = 0;
#endif
    zval *key = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O|sz",
                              &db, php_sp_db_ce,
                              &order, &order_len, &key) == FAILURE) {
        RETURN_FALSE;
    }

    php_sp_cursor_create(getThis(), db, order, order_len, key TSRMLS_CC);
}

PHP_SOPHIA_METHOD(Cursor, rewind)
{
    php_sp_cursor_t *intern;
    void *object;
#ifdef ZEND_ENGINE_3
    zend_long cnt;
#else
    long cnt;
#endif

    if (zend_parse_parameters_none() == FAILURE) {
        RETURN_FALSE;
    }

    intern = PHP_SP_CURSOR_OBJ(getThis());
    if (!intern) {
        RETURN_FALSE;
    }

    if (intern->sophia.current) {
        sp_destroy(intern->sophia.current);
        intern->sophia.current = NULL;
    }

    object = sp_object(intern->sophia.db);

    if (!intern->first && intern->sophia.cursor) {
        sp_destroy(intern->sophia.cursor);
        if (intern->order.c) {
            if (sp_set(object, "order", intern->order.c) == -1) {
                PHP_SP_ERR(E_WARNING, "Error iteration order set");
            }
        }
#ifdef ZEND_ENGINE_3
        cnt = zend_array_count(Z_ARRVAL(intern->key));
#else
        cnt = zend_hash_num_elements(Z_ARRVAL_P(intern->key));
#endif
        if (cnt > 0) {
#ifdef ZEND_ENGINE_3
            zend_string *str_key;
            ulong index_key;
            zval *index;

            ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(intern->key),
                                      index_key, str_key, index) {
                switch (Z_TYPE_P(index)) {
                    case IS_STRING:
                        sp_set(object, ZSTR_VAL(str_key),
                               Z_STRVAL_P(index), Z_STRLEN_P(index));
                        break;
                    case IS_LONG:
                        sp_set(object, ZSTR_VAL(str_key),
                               &Z_LVAL_P(index), sizeof(Z_LVAL_P(index)));
                        break;
                    default:
                        PHP_SP_ERR(E_WARNING, "Invalid key type");
                }
            } ZEND_HASH_FOREACH_END();
#else
            zval **index;
            uint str_key_len;
            char *str_key;
            ulong index_key;

            for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(intern->key));
                 zend_hash_get_current_data(Z_ARRVAL_P(intern->key),
                                            (void *)&index) == SUCCESS;
                 zend_hash_move_forward(Z_ARRVAL_P(intern->key))) {
                if (zend_hash_get_current_key_ex(
                        Z_ARRVAL_P(intern->key), &str_key, &str_key_len,
                        &index_key, 0, NULL) == HASH_KEY_IS_STRING) {
                    switch (Z_TYPE_PP(index)) {
                        case IS_STRING:
                            sp_set(object, str_key,
                                   Z_STRVAL_PP(index), Z_STRLEN_PP(index));
                            break;
                        case IS_LONG:
                            sp_set(object, str_key,
                                   &Z_LVAL_PP(index), sizeof(Z_LVAL_PP(index)));
                            break;
                        default:
                            PHP_SP_ERR(E_WARNING, "Invalid key type");
                    }
                }
            }
#endif
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

    intern = PHP_SP_CURSOR_OBJ(getThis());
    if (!intern) {
        RETURN_FALSE;
    }

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

    intern = PHP_SP_CURSOR_OBJ(getThis());
    if (!intern) {
        RETURN_FALSE;
    }

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
    void *object;

    if (zend_parse_parameters_none() == FAILURE) {
        RETURN_FALSE;
    }

    intern = PHP_SP_CURSOR_OBJ(getThis());
    if (!intern) {
        RETURN_FALSE;
    }

    if (!intern->sophia.current) {
        object = sp_object(intern->sophia.db);
        intern->sophia.current = sp_get(intern->sophia.cursor, object);
        if (!intern->sophia.current) {
            RETURN_FALSE;
        }
    }

    value = sp_get(intern->sophia.current, "value", &value_len);

    if (value_len == 0) {
        RETURN_EMPTY_STRING();
    } else if (value) {
#ifdef ZEND_ENGINE_3
        RETURN_STRINGL(value, value_len);
#else
        RETURN_STRINGL(value, value_len, 1);
#endif
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

    intern = PHP_SP_CURSOR_OBJ(getThis());
    if (!intern) {
        RETURN_FALSE;
    }

    if (!intern->sophia.current) {
        RETURN_FALSE;
    }

    key = sp_get(intern->sophia.current, "key", &key_len);

    if (key_len == 0) {
        RETURN_EMPTY_STRING();
    } else if (key) {
#ifdef ZEND_ENGINE_3
        RETURN_STRINGL(key, key_len);
#else
        RETURN_STRINGL(key, key_len, 1);
#endif
    } else {
        RETURN_FALSE;
    }
}

PHP_SOPHIA_METHOD(Cursor, keys)
{
    php_sp_cursor_t *intern;
    char *key = NULL;
    int key_len = 0;
#ifdef ZEND_ENGINE_3
    zval zv;
    zend_long cnt;
#else
    zval* zv;
    long cnt;
#endif

    if (zend_parse_parameters_none() == FAILURE) {
        RETURN_FALSE;
    }

    intern = PHP_SP_CURSOR_OBJ(getThis());
    if (!intern) {
        RETURN_FALSE;
    }

    if (!intern->sophia.current) {
        RETURN_FALSE;
    }

    key = sp_get(intern->sophia.current, "key", &key_len);

    array_init(return_value);

#ifdef ZEND_ENGINE_3
    ZVAL_STRINGL(&zv, key, key_len);
    zend_hash_str_add(Z_ARRVAL_P(return_value), "key", sizeof("key")-1, &zv);

    cnt = zend_array_count(Z_ARRVAL(intern->key));
#else
    MAKE_STD_ZVAL(zv);
    ZVAL_STRINGL(zv, key, key_len, 1);
    zend_hash_add(Z_ARRVAL_P(return_value),
                  "key", sizeof("key"), &zv, sizeof(zval*), NULL);

    cnt = zend_hash_num_elements(Z_ARRVAL_P(intern->key));
#endif

    if (cnt > 0) {
#ifdef ZEND_ENGINE_3
        zend_string *str_key;
        ulong index_key;
        zval *index;

        ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(intern->key),
                                  index_key, str_key, index) {
            switch (Z_TYPE_P(index)) {
                case IS_STRING:
                    key = sp_get(intern->sophia.current,
                                 ZSTR_VAL(str_key), &key_len);
                    ZVAL_STRINGL(&zv, key, key_len);
                    zend_hash_add(Z_ARRVAL_P(return_value), str_key, &zv);
                    break;
                case IS_LONG: {
                    uint32_t *n;
                    n = sp_get(intern->sophia.current,
                               ZSTR_VAL(str_key), &key_len);
                    ZVAL_LONG(&zv, *n);
                    zend_hash_add(Z_ARRVAL_P(return_value), str_key, &zv);
                    break;
                }
            }
        } ZEND_HASH_FOREACH_END();
#else
        zval **index;
        uint str_key_len;
        char *str_key;
        ulong index_key;

        for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(intern->key));
             zend_hash_get_current_data(Z_ARRVAL_P(intern->key),
                                        (void *)&index) == SUCCESS;
             zend_hash_move_forward(Z_ARRVAL_P(intern->key))) {
            if (zend_hash_get_current_key_ex(
                    Z_ARRVAL_P(intern->key), &str_key, &str_key_len,
                    &index_key, 0, NULL) == HASH_KEY_IS_STRING) {
                switch (Z_TYPE_PP(index)) {
                    case IS_STRING:
                        key = sp_get(intern->sophia.current, str_key, &key_len);
                        MAKE_STD_ZVAL(zv);
                        ZVAL_STRINGL(zv, key, key_len, 1);
                        zend_hash_add(Z_ARRVAL_P(return_value),
                                      str_key, str_key_len,
                                      &zv, sizeof(zval*), NULL);
                        break;
                    case IS_LONG: {
                        uint32_t *n;
                        n = sp_get(intern->sophia.current, str_key, &key_len);
                        MAKE_STD_ZVAL(zv);
                        ZVAL_LONG(zv, *n);
                        zend_hash_add(Z_ARRVAL_P(return_value),
                                      str_key, str_key_len,
                                      &zv, sizeof(zval*), NULL);
                        break;
                    }
                }
            }
        }
#endif
    }
}

PHP_SOPHIA_API void
php_sp_cursor_construct(zval *return_value, zval *db,
                        char *order, int order_len, zval *key TSRMLS_DC)
{
    object_init_ex(return_value, php_sp_cursor_ce);
    php_sp_cursor_create(return_value, db, order, order_len, key TSRMLS_CC);
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
    PHP_SOPHIA_ME(Cursor, keys,
                  arginfo_sp_cursor_no_parameters, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

#ifdef ZEND_ENGINE_3
static inline void
php_sp_cursor_free_storage(zend_object *std)
{
    php_sp_cursor_t *intern;

    intern = (php_sp_cursor_t *)
        ((char *)std - XtOffsetOf(php_sp_cursor_t, std));
    if (!intern) {
        return;
    }

    smart_string_free(&intern->order);
    zval_ptr_dtor(&intern->key);

    if (intern->sophia.current) {
        sp_destroy(intern->sophia.current);
    }

    if (intern->sophia.cursor &&
        php_sp_db_object_get_database(&intern->db TSRMLS_CC)) {
        sp_destroy(intern->sophia.cursor);
    }

    zval_ptr_dtor(&intern->db);

    zend_object_std_dtor(std);
}

static inline zend_object *
php_sp_cursor_new_ex(zend_class_entry *ce, php_sp_cursor_t **ptr TSRMLS_DC)
{
    php_sp_cursor_t *intern;

    intern = ecalloc(1,
                     sizeof(php_sp_cursor_t) + zend_object_properties_size(ce));
    if (ptr) {
        *ptr = intern;
    }

    zend_object_std_init(&intern->std, ce);
    object_properties_init(&intern->std, ce);
    rebuild_object_properties(&intern->std);

    intern->std.handlers = &php_sp_cursor_handlers;

    return &intern->std;
}

static inline zend_object *
php_sp_cursor_new(zend_class_entry *ce TSRMLS_DC)
{
    return php_sp_cursor_new_ex(ce, NULL TSRMLS_CC);
}
#else
static inline void
php_sp_cursor_free_storage(void *object TSRMLS_DC)
{
    php_sp_cursor_t *intern = (php_sp_cursor_t *)object;

    if (!intern) {
        return;
    }

    smart_str_free(&intern->order);
    zval_ptr_dtor(&intern->key);

    if (intern->sophia.current) {
        sp_destroy(intern->sophia.current);
    }

    if (intern->sophia.cursor &&
        php_sp_db_object_get_database(intern->db TSRMLS_CC)) {
        sp_destroy(intern->sophia.cursor);
    }

    if (intern->db) {
        zval_ptr_dtor(&intern->db);
    }

    zend_object_std_dtor(&intern->std TSRMLS_CC);
    efree(object);
}

static inline zend_object_value
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

static inline zend_object_value
php_sp_cursor_new(zend_class_entry *ce TSRMLS_DC)
{
    return php_sp_cursor_new_ex(ce, NULL TSRMLS_CC);
}
#endif

PHP_SOPHIA_API int
php_sp_cursor_class_register(TSRMLS_D)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, ZEND_NS_NAME(PHP_SOPHIA_NS, "Cursor"),
                     php_sp_cursor_methods);

    php_sp_cursor_ce = zend_register_internal_class(&ce TSRMLS_CC);
    zend_class_implements(php_sp_cursor_ce TSRMLS_CC, 1, zend_ce_iterator);

    php_sp_cursor_ce->create_object = php_sp_cursor_new;

    memcpy(&php_sp_cursor_handlers, &std_object_handlers,
           sizeof(zend_object_handlers));
#ifdef ZEND_ENGINE_3
    php_sp_cursor_handlers.offset = XtOffsetOf(php_sp_cursor_t, std);
    php_sp_cursor_handlers.free_obj = php_sp_cursor_free_storage;
#endif
    php_sp_cursor_handlers.clone_obj = NULL;

    return SUCCESS;
}
