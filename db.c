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

#include "php_sophia.h"
#include "db.h"
#include "cursor.h"
#include "exception.h"

zend_class_entry *php_sp_db_ce;
static zend_object_handlers php_sp_db_handlers;

ZEND_EXTERN_MODULE_GLOBALS(sophia)

ZEND_BEGIN_ARG_INFO_EX(arginfo_sp_db___construct, 0, ZEND_RETURN_VALUE, 1)
    ZEND_ARG_INFO(0, db)
    ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sp_db_set, 0, ZEND_RETURN_VALUE, 2)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sp_db_get, 0, ZEND_RETURN_VALUE, 1)
    ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sp_db_delete, 0, ZEND_RETURN_VALUE, 1)
    ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sp_db_begin, 0, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sp_db_commit, 0, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sp_db_rollback, 0, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sp_db_drop, 0, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sp_db_close, 0, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sp_db_cursor, 0, ZEND_RETURN_VALUE, 0)
    ZEND_ARG_INFO(0, order)
    ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

PHP_SOPHIA_METHOD(Db, __construct)
{
    php_sp_db_t *intern;
    char *db, *db_name = NULL, *log_path = NULL, *sophia_path = NULL;
#ifdef ZEND_ENGINE_3
    size_t db_len;
#else
    int db_len;
#endif
    zval *options = NULL;
    zend_bool log_path_free = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|a",
                              &db, &db_len, &options) == FAILURE) {
        RETURN_FALSE;
    }

    if (!db || db_len == 0) {
        PHP_SP_EXCEPTION(0, "no such database");
        RETURN_FALSE;
    }

    intern = PHP_SP_DB_OBJ_NOCHECK(getThis());
    if (!intern) {
        PHP_SP_EXCEPTION(0, "Can not database object");
        RETURN_FALSE;
    }

    /* get path options */
    if (options) {
#ifdef ZEND_ENGINE_3
        zval *path;
        if ((path = zend_hash_str_find(Z_ARRVAL_P(options), "sophia.path",
                                       sizeof("sophia.path")-1)) != NULL) {
            sophia_path = Z_STRVAL_P(path);
        }

        if ((path = zend_hash_str_find(Z_ARRVAL_P(options), "log.path",
                                       sizeof("log.path")-1)) != NULL) {
            log_path = Z_STRVAL_P(path);
        }
#else
        zval **path;
        if (zend_hash_find(Z_ARRVAL_P(options),
                           "sophia.path", sizeof("sophia.path"),
                           (void **)&path) != FAILURE && path) {
            sophia_path = Z_STRVAL_PP(path);
        }

        if (zend_hash_find(Z_ARRVAL_P(options),
                           "log.path", sizeof("log.path"),
                           (void **)&path) != FAILURE && path) {
            log_path = Z_STRVAL_PP(path);
        }
#endif
    }

    /* create sophia environment */
    intern->env = sp_env();
    intern->ctl = sp_ctl(intern->env);

    if (!sophia_path || strlen(sophia_path) == 0) {
        sophia_path = PHP_SOPHIA_G(path);
        if (!sophia_path || strlen(sophia_path) == 0) {
            sophia_path = PHP_SOPHIA_DEFAULT_PATH;
        }
    }

    if (php_check_open_basedir(sophia_path TSRMLS_CC) ) {
        RETURN_FALSE;
    }

    sp_set(intern->ctl, "sophia.path", sophia_path);

    if (!log_path) {
        log_path = (char *)emalloc(strlen(sophia_path) + db_len + 6);
        if (!log_path) {
            PHP_SP_EXCEPTION(0, "memory allocate");
            RETURN_FALSE;
        }
        php_sprintf(log_path, "%s/%.*s.log", sophia_path, db_len, db);
        log_path_free = 1;
    }

    if (php_check_open_basedir(log_path TSRMLS_CC) ) {
        RETURN_FALSE;
    }
    sp_set(intern->ctl, "log.path", log_path);

    /* create/open database */
    db_name = (char *)emalloc(db_len + 4);
    if (!db_name) {
        if (log_path && log_path_free) {
            efree(log_path);
        }
        PHP_SP_EXCEPTION(0, "memory allocate");
        RETURN_FALSE;
    }
    php_sprintf(db_name, "db.%.*s", db_len, db);

    sp_set(intern->ctl, "db", db);

    /* set options */
    if (options) {
#ifdef ZEND_ENGINE_3
        zend_string *key;
        ulong index_key;
        zval *value;
        ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(options), index_key, key, value) {
            if (key && strcmp(ZSTR_VAL(key), "sophia.path") != 0
                && strcmp(ZSTR_VAL(key), "log.path") != 0) {
                if (sp_set(intern->ctl, ZSTR_VAL(key),
                           Z_STRVAL_P(value)) == -1) {
                    PHP_SP_ERR(E_WARNING,
                               "Error set option: %s", ZSTR_VAL(key));
                }
            }
        } ZEND_HASH_FOREACH_END();
#else
        zval **value;
        uint key_len;
        char *key;
        ulong index_key;

        for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(options));
             zend_hash_get_current_data(Z_ARRVAL_P(options),
                                        (void *)&value) == SUCCESS;
             zend_hash_move_forward(Z_ARRVAL_P(options))) {
            if (zend_hash_get_current_key_ex(Z_ARRVAL_P(options),
                                             &key, &key_len, &index_key,
                                             0, NULL) == HASH_KEY_IS_STRING
                && Z_TYPE_PP(value) == IS_STRING
                && strcmp(key, "sophia.path") != 0
                && strcmp(key, "log.path") != 0) {
                if (sp_set(intern->ctl, key, Z_STRVAL_PP(value)) == -1) {
                    PHP_SP_ERR(E_WARNING, "Error set option: %s", key);
                }
            }
        }
#endif
    }

    if (sp_open(intern->env) == -1) {
        efree(db_name);
        if (log_path && log_path_free) {
            efree(log_path);
        }
        PHP_SP_EXCEPTION(0, "create or open database error: %s", db);
        RETURN_FALSE;
    }

    intern->db = sp_get(intern->ctl, db_name);
    if (!intern->db) {
        efree(db_name);
        if (log_path && log_path_free) {
            efree(log_path);
        }
        PHP_SP_EXCEPTION(0, "create or open database error: %s", db);
        RETURN_FALSE;
    }

    efree(db_name);
    if (log_path && log_path_free) {
        efree(log_path);
    }
}

PHP_SOPHIA_METHOD(Db, set)
{
    php_sp_db_t *intern;
    zval *key = NULL;
    char *value = NULL;
#ifdef ZEND_ENGINE_3
    size_t value_len = 0;
#else
    int value_len = 0;
#endif
    void *object;


    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zs",
                              &key, &value, &value_len) == FAILURE) {
        RETURN_FALSE;
    }

    intern = PHP_SP_DB_OBJ(getThis());
    if (!intern) {
        RETURN_FALSE;
    }

    object = sp_object(intern->db);

    switch (Z_TYPE_P(key)) {
        default:
            convert_to_string(key);
        case IS_STRING:
            sp_set(object, "key", Z_STRVAL_P(key), Z_STRLEN_P(key));
            break;
        case IS_ARRAY: {
#ifdef ZEND_ENGINE_3
            zend_string *str_key;
            ulong index_key;
            zval *index;
            ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(key),
                                      index_key, str_key, index) {
                if (key) {
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
                if (zend_hash_get_current_key_ex(Z_ARRVAL_P(key),
                                                 &str_key, &str_key_len,
                                                 &index_key, 0,
                                                 NULL) == HASH_KEY_IS_STRING) {
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
            break;
        }
    }

    sp_set(object, "value", value, value_len);

    if (intern->transaction) {
        if (sp_set(intern->transaction, object) == -1) {
            PHP_SP_ERR(E_WARNING, "Error set key: %s", key);
            RETURN_FALSE;
        }
    } else {
        if (sp_set(intern->db, object) == -1) {
            PHP_SP_ERR(E_WARNING, "Error set key: %s", key);
            RETURN_FALSE;
        }
    }

    RETURN_TRUE;
}

PHP_SOPHIA_METHOD(Db, get)
{
    php_sp_db_t *intern = NULL;
    zval *key = NULL;
    char *value = NULL;
    int value_len = 0;
    void *object, *result;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z",
                              &key) == FAILURE) {
        RETURN_FALSE;
    }

    intern = PHP_SP_DB_OBJ(getThis());
    if (!intern) {
        RETURN_FALSE;
    }

    object = sp_object(intern->db);

    switch (Z_TYPE_P(key)) {
        default:
            convert_to_string(key);
        case IS_STRING:
            sp_set(object, "key", Z_STRVAL_P(key), Z_STRLEN_P(key));
            break;
        case IS_ARRAY: {
#ifdef ZEND_ENGINE_3
            zend_string *str_key;
            ulong index_key;
            zval *index;
            ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(key),
                                      index_key, str_key, index) {
                if (key) {
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
                if (zend_hash_get_current_key_ex(Z_ARRVAL_P(key),
                                                 &str_key, &str_key_len,
                                                 &index_key, 0,
                                                 NULL) == HASH_KEY_IS_STRING) {
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
            break;
        }
    }

    if (intern->transaction) {
        result = sp_get(intern->transaction, object);
    } else {
        result = sp_get(intern->db, object);
    }

    if (result) {
        value = sp_get(result, "value", &value_len);
        if (value_len == 0) {
            RETVAL_EMPTY_STRING();
        } else if (value) {
#ifdef ZEND_ENGINE_3
            RETVAL_STRINGL(value, value_len);
#else
            RETVAL_STRINGL(value, value_len, 1);
#endif
        } else {
            RETVAL_FALSE;
        }
        sp_destroy(result);
    } else {
        RETVAL_FALSE;
    }
}

PHP_SOPHIA_METHOD(Db, delete)
{
    php_sp_db_t *intern;
    zval *key;
    void *object;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z",
                              &key) == FAILURE) {
        RETURN_FALSE;
    }

    intern = PHP_SP_DB_OBJ(getThis());
    if (!intern) {
        RETURN_FALSE;
    }

    object = sp_object(intern->db);

    switch (Z_TYPE_P(key)) {
        default:
            convert_to_string(key);
        case IS_STRING:
            sp_set(object, "key", Z_STRVAL_P(key), Z_STRLEN_P(key));
            break;
        case IS_ARRAY: {
#ifdef ZEND_ENGINE_3
            zend_string *str_key;
            ulong index_key;
            zval *index;
            ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(key),
                                      index_key, str_key, index) {
                if (key) {
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
                if (zend_hash_get_current_key_ex(Z_ARRVAL_P(key),
                                                 &str_key, &str_key_len,
                                                 &index_key, 0,
                                                 NULL) == HASH_KEY_IS_STRING) {
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
            break;
        }
    }

    if (intern->transaction) {
        if (sp_delete(intern->transaction, object) == -1) {
            RETURN_FALSE;
        }
    } else {
        if (sp_delete(intern->db, object) == -1) {
            RETURN_FALSE;
        }
    }

    RETURN_TRUE;
}

PHP_SOPHIA_METHOD(Db, begin)
{
    php_sp_db_t *intern;

    if (zend_parse_parameters_none() == FAILURE) {
        RETURN_FALSE;
    }

    intern = PHP_SP_DB_OBJ(getThis());
    if (!intern) {
        RETURN_FALSE;
    }

    if (intern->transaction) {
        PHP_SP_ERR(E_WARNING, "Already transaction");
        RETURN_FALSE;
    }

    intern->transaction = sp_begin(intern->env);
    if (intern->transaction == NULL) {
        PHP_SP_ERR(E_WARNING, "Error start transaction");
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_SOPHIA_METHOD(Db, commit)
{
    php_sp_db_t *intern;

    if (zend_parse_parameters_none() == FAILURE) {
        RETURN_FALSE;
    }

    intern = PHP_SP_DB_OBJ(getThis());
    if (!intern) {
        RETURN_FALSE;
    }

    if (!intern->transaction) {
        PHP_SP_ERR(E_WARNING, "Can not start transaction");
        RETURN_FALSE;
    }

    if (sp_commit(intern->transaction) == -1) {
        PHP_SP_ERR(E_WARNING, "Error transaction commit");
        RETURN_FALSE;
    }

    intern->transaction = NULL;

    RETURN_TRUE;
}

PHP_SOPHIA_METHOD(Db, rollback)
{
    php_sp_db_t *intern;

    if (zend_parse_parameters_none() == FAILURE) {
        RETURN_FALSE;
    }

    intern = PHP_SP_DB_OBJ(getThis());
    if (!intern) {
        RETURN_FALSE;
    }

    if (!intern->transaction) {
        PHP_SP_ERR(E_WARNING, "Can not start transaction");
        RETURN_FALSE;
    }

    if (sp_destroy(intern->transaction) == -1) {
        PHP_SP_ERR(E_WARNING, "Error transaction rollback");
        RETURN_FALSE;
    }

    intern->transaction = NULL;

    RETURN_TRUE;
}

PHP_SOPHIA_METHOD(Db, close)
{
    php_sp_db_t *intern;

    if (zend_parse_parameters_none() == FAILURE) {
        RETURN_FALSE;
    }

    intern = PHP_SP_DB_OBJ(getThis());
    if (!intern) {
        RETURN_FALSE;
    }

    if (intern->env) {
        if (sp_destroy(intern->env) == -1) {
            PHP_SP_ERR(E_WARNING, "Error db close");
        }
    }

    intern->env = NULL;
    intern->ctl = NULL;
    intern->db = NULL;
    intern->transaction = NULL;

    RETURN_TRUE;
}

PHP_SOPHIA_METHOD(Db, drop)
{
    php_sp_db_t *intern;
    void *object;

    if (zend_parse_parameters_none() == FAILURE) {
        RETURN_FALSE;
    }

    intern = PHP_SP_DB_OBJ(getThis());
    if (!intern) {
        RETURN_FALSE;
    }

    if (sp_drop(intern->db) == -1) {
        RETURN_FALSE;
    }

    intern->db = NULL;

    RETURN_TRUE;
}

PHP_SOPHIA_METHOD(Db, cursor)
{
    php_sp_db_t *intern;
    char *order = NULL;
#ifdef ZEND_ENGINE_3
    size_t order_len = 0;
#else
    int order_len = 0;
#endif
    zval *key = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|sz",
                              &order, &order_len, &key) == FAILURE) {
        RETURN_FALSE;
    }

    intern = PHP_SP_DB_OBJ(getThis());
    if (!intern) {
        RETURN_FALSE;
    }

    php_sp_cursor_construct(return_value, getThis(),
                            order, order_len, key TSRMLS_CC);
}

PHP_SOPHIA_API void *
php_sp_db_object_get_database(zval *db TSRMLS_DC)
{
    php_sp_db_t *intern;

    if (!db) {
        return NULL;
    }

    intern = PHP_SP_DB_OBJ_NOCHECK(db);

    if (intern && intern->db) {
        return intern->db;
    } else {
        return NULL;
    }
}


static zend_function_entry php_sp_db_methods[] = {
    PHP_SOPHIA_ME(Db, __construct, arginfo_sp_db___construct,
                  ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_SOPHIA_ME(Db, set, arginfo_sp_db_set, ZEND_ACC_PUBLIC)
    PHP_SOPHIA_ME(Db, get, arginfo_sp_db_get, ZEND_ACC_PUBLIC)
    PHP_SOPHIA_ME(Db, delete, arginfo_sp_db_delete, ZEND_ACC_PUBLIC)
    PHP_SOPHIA_ME(Db, begin, arginfo_sp_db_begin, ZEND_ACC_PUBLIC)
    PHP_SOPHIA_ME(Db, commit, arginfo_sp_db_commit, ZEND_ACC_PUBLIC)
    PHP_SOPHIA_ME(Db, rollback, arginfo_sp_db_rollback, ZEND_ACC_PUBLIC)
    PHP_SOPHIA_ME(Db, close, arginfo_sp_db_close, ZEND_ACC_PUBLIC)
    PHP_SOPHIA_ME(Db, drop, arginfo_sp_db_drop, ZEND_ACC_PUBLIC)
    PHP_SOPHIA_ME(Db, cursor, arginfo_sp_db_cursor, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

#ifdef ZEND_ENGINE_3
static inline void
php_sp_db_free_storage(zend_object *std)
{
    php_sp_db_t *intern;

    intern = (php_sp_db_t *)((char *)std - XtOffsetOf(php_sp_db_t, std));
    if (!intern) {
        return;
    }

    if (intern->env) {
        sp_destroy(intern->env);
    }

    zend_object_std_dtor(std);
}

static inline zend_object *
php_sp_db_new_ex(zend_class_entry *ce, php_sp_db_t **ptr TSRMLS_DC)
{
    php_sp_db_t *intern;

    intern = ecalloc(1, sizeof(php_sp_db_t) + zend_object_properties_size(ce));
    if (ptr) {
        *ptr = intern;
    }

    zend_object_std_init(&intern->std, ce);
    object_properties_init(&intern->std, ce);
    rebuild_object_properties(&intern->std);

    intern->std.handlers = &php_sp_db_handlers;

    return &intern->std;
}

static inline zend_object *
php_sp_db_new(zend_class_entry *ce TSRMLS_DC)
{
    return php_sp_db_new_ex(ce, NULL TSRMLS_CC);
}
#else
static inline void
php_sp_db_free_storage(void *object TSRMLS_DC)
{
    php_sp_db_t *intern = (php_sp_db_t *)object;

    if (!intern) {
        return;
    }

    if (intern->env) {
        sp_destroy(intern->env);
    }

    zend_object_std_dtor(&intern->std TSRMLS_CC);
    efree(object);
}

static inline zend_object_value
php_sp_db_new_ex(zend_class_entry *ce, php_sp_db_t **ptr TSRMLS_DC)
{
    php_sp_db_t *intern;
    zend_object_value retval;
#if ZEND_MODULE_API_NO < 20100525
    zval *tmp;
#endif

    intern = (php_sp_db_t *)emalloc(sizeof(php_sp_db_t));
    memset(intern, 0, sizeof(php_sp_db_t));
    if (ptr) {
        *ptr = intern;
    }

    zend_object_std_init(&intern->std, ce TSRMLS_CC);
#if ZEND_MODULE_API_NO >= 20100525
    object_properties_init(&intern->std, ce);
#else
    zend_hash_copy(intern->std.properties, &ce->default_properties,
                   (copy_ctor_func_t)zval_add_ref, (void *)&tmp,
                   sizeof(zval *));
#endif

    retval.handle = zend_objects_store_put(
        intern, (zend_objects_store_dtor_t)zend_objects_destroy_object,
        (zend_objects_free_object_storage_t)php_sp_db_free_storage,
        NULL TSRMLS_CC);
    retval.handlers = &php_sp_db_handlers;

    return retval;
}

static zend_object_value
php_sp_db_new(zend_class_entry *ce TSRMLS_DC)
{
    return php_sp_db_new_ex(ce, NULL TSRMLS_CC);
}
#endif

PHP_SOPHIA_API int
php_sp_db_class_register(TSRMLS_D)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, ZEND_NS_NAME(PHP_SOPHIA_NS, "Db"), php_sp_db_methods);

    php_sp_db_ce = zend_register_internal_class(&ce TSRMLS_CC);
    php_sp_db_ce->create_object = php_sp_db_new;

    memcpy(&php_sp_db_handlers, &std_object_handlers,
           sizeof(zend_object_handlers));
#ifdef ZEND_ENGINE_3
    php_sp_db_handlers.offset = XtOffsetOf(php_sp_db_t, std);
    php_sp_db_handlers.dtor_obj = zend_objects_destroy_object;
    php_sp_db_handlers.free_obj = php_sp_db_free_storage;
#endif
    php_sp_db_handlers.clone_obj = NULL;

    return SUCCESS;
}
