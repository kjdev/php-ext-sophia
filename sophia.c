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
#include "config.h"
#endif

#include <php.h>
#include <php_ini.h>
#include <ext/standard/info.h>
#include <Zend/zend_exceptions.h>

#include "php_sophia.h"
#include "db.h"
#include "cursor.h"
#include "exception.h"

ZEND_DECLARE_MODULE_GLOBALS(sophia)

ZEND_INI_BEGIN()
    STD_ZEND_INI_ENTRY("sophia.path", PHP_SOPHIA_DEFAULT_PATH,
                       ZEND_INI_ALL, OnUpdateString, path,
                       zend_sophia_globals, sophia_globals)
ZEND_INI_END()

static void
php_sophia_init_globals(zend_sophia_globals *sophia_globals)
{
    sophia_globals->path = PHP_SOPHIA_DEFAULT_PATH;
}

ZEND_MINIT_FUNCTION(sophia)
{
    php_sp_exception_class_register(TSRMLS_C);
    php_sp_db_class_register(TSRMLS_C);
    php_sp_cursor_class_register(TSRMLS_C);

    ZEND_INIT_MODULE_GLOBALS(sophia, php_sophia_init_globals, NULL);
    REGISTER_INI_ENTRIES();

    return SUCCESS;
}

ZEND_MSHUTDOWN_FUNCTION(sophia)
{
    UNREGISTER_INI_ENTRIES();
    return SUCCESS;
}

ZEND_MINFO_FUNCTION(sophia)
{
    php_info_print_table_start();
    php_info_print_table_row(2, "Sophia support", "enabled");
    php_info_print_table_row(2, "Extension Version", PHP_SOPHIA_EXT_VERSION);
    php_info_print_table_end();

    DISPLAY_INI_ENTRIES();
}

zend_module_entry sophia_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    "sophia",
    NULL,
    ZEND_MINIT(sophia),
    ZEND_MSHUTDOWN(sophia),
    NULL,
    NULL,
    ZEND_MINFO(sophia),
#if ZEND_MODULE_API_NO >= 20010901
    PHP_SOPHIA_EXT_VERSION,
#endif
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_SOPHIA
ZEND_GET_MODULE(sophia)
#endif
