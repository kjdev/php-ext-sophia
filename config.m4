dnl config.m4 for extension sophia

dnl Check PHP version:
AC_MSG_CHECKING(PHP version)
if test ! -z "$phpincludedir"; then
    PHP_VERSION=`grep 'PHP_VERSION ' $phpincludedir/main/php_version.h | sed -e 's/.*"\([[0-9\.]]*\)".*/\1/g' 2>/dev/null`
elif test ! -z "$PHP_CONFIG"; then
    PHP_VERSION=`$PHP_CONFIG --version 2>/dev/null`
fi

if test x"$PHP_VERSION" = "x"; then
    AC_MSG_WARN([none])
else
    PHP_MAJOR_VERSION=`echo $PHP_VERSION | sed -e 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*/\1/g' 2>/dev/null`
    PHP_MINOR_VERSION=`echo $PHP_VERSION | sed -e 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*/\2/g' 2>/dev/null`
    PHP_RELEASE_VERSION=`echo $PHP_VERSION | sed -e 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*/\3/g' 2>/dev/null`
    AC_MSG_RESULT([$PHP_VERSION])
fi

if test $PHP_MAJOR_VERSION -lt 5; then
    AC_MSG_ERROR([need at least PHP 5 or newer])
fi

PHP_ARG_ENABLE(sophia, whether to enable sophia support,
[  --enable-sophia           Enable sophia support])

if test "$PHP_SOPHIA" != "no"; then

    PHP_ARG_ENABLE(sophia-embedded, whether to enable sophia embedded,
    [  --enable-sophia-embedded   Enable sophia embedded], yes, no)

    if test "$PHP_SOPHIA_EMBEDDED" != "no"; then

        dnl Embedded sophia

        SOPHIA_DIR="sophia/sophia"

        PHP_ADD_INCLUDE("$SOPHIA_DIR/std")
        PHP_ADD_INCLUDE("$SOPHIA_DIR/format")
        PHP_ADD_INCLUDE("$SOPHIA_DIR/runtime")
        PHP_ADD_INCLUDE("$SOPHIA_DIR/object")
        PHP_ADD_INCLUDE("$SOPHIA_DIR/version")
        PHP_ADD_INCLUDE("$SOPHIA_DIR/log")
        PHP_ADD_INCLUDE("$SOPHIA_DIR/database")
        PHP_ADD_INCLUDE("$SOPHIA_DIR/index")
        PHP_ADD_INCLUDE("$SOPHIA_DIR/transaction")
        PHP_ADD_INCLUDE("$SOPHIA_DIR/repository")
        PHP_ADD_INCLUDE("$SOPHIA_DIR/environment")
        PHP_ADD_INCLUDE("$SOPHIA_DIR/sophia")

        SOPHIA_SOURCES="$SOPHIA_DIR/std/ss_bufiter.c $SOPHIA_DIR/std/ss_crc.c $SOPHIA_DIR/std/ss_lz4filter.c $SOPHIA_DIR/std/ss_nonefilter.c $SOPHIA_DIR/std/ss_ooma.c $SOPHIA_DIR/std/ss_pager.c $SOPHIA_DIR/std/ss_qf.c $SOPHIA_DIR/std/ss_quota.c $SOPHIA_DIR/std/ss_rb.c $SOPHIA_DIR/std/ss_slaba.c $SOPHIA_DIR/std/ss_stda.c $SOPHIA_DIR/std/ss_stdvfs.c $SOPHIA_DIR/std/ss_testvfs.c $SOPHIA_DIR/std/ss_thread.c $SOPHIA_DIR/std/ss_time.c $SOPHIA_DIR/std/ss_trigger.c $SOPHIA_DIR/std/ss_zstdfilter.c $SOPHIA_DIR/format/sf.c $SOPHIA_DIR/runtime/sr_conf.c $SOPHIA_DIR/runtime/sr_scheme.c $SOPHIA_DIR/object/so.c $SOPHIA_DIR/version/sv_index.c $SOPHIA_DIR/version/sv_indexiter.c $SOPHIA_DIR/version/sv_mergeiter.c $SOPHIA_DIR/version/sv_readiter.c $SOPHIA_DIR/version/sv_ref.c $SOPHIA_DIR/version/sv_upsertv.c $SOPHIA_DIR/version/sv_v.c $SOPHIA_DIR/version/sv_writeiter.c $SOPHIA_DIR/log/sl.c $SOPHIA_DIR/log/sl_dir.c $SOPHIA_DIR/log/sl_iter.c $SOPHIA_DIR/log/sl_v.c $SOPHIA_DIR/database/sd_build.c $SOPHIA_DIR/database/sd_index.c $SOPHIA_DIR/database/sd_indexiter.c $SOPHIA_DIR/database/sd_iter.c $SOPHIA_DIR/database/sd_merge.c $SOPHIA_DIR/database/sd_pageiter.c $SOPHIA_DIR/database/sd_read.c $SOPHIA_DIR/database/sd_recover.c $SOPHIA_DIR/database/sd_scheme.c $SOPHIA_DIR/database/sd_schemeiter.c $SOPHIA_DIR/database/sd_snapshot.c $SOPHIA_DIR/database/sd_snapshotiter.c $SOPHIA_DIR/database/sd_v.c $SOPHIA_DIR/database/sd_write.c $SOPHIA_DIR/index/si.c $SOPHIA_DIR/index/si_anticache.c $SOPHIA_DIR/index/si_backup.c $SOPHIA_DIR/index/si_compaction.c $SOPHIA_DIR/index/si_drop.c $SOPHIA_DIR/index/si_gc.c $SOPHIA_DIR/index/si_iter.c $SOPHIA_DIR/index/si_merge.c $SOPHIA_DIR/index/si_node.c $SOPHIA_DIR/index/si_planner.c $SOPHIA_DIR/index/si_profiler.c $SOPHIA_DIR/index/si_read.c $SOPHIA_DIR/index/si_recover.c $SOPHIA_DIR/index/si_scheme.c $SOPHIA_DIR/index/si_snapshot.c $SOPHIA_DIR/index/si_tx.c $SOPHIA_DIR/index/si_write.c $SOPHIA_DIR/transaction/sx.c $SOPHIA_DIR/transaction/sx_deadlock.c $SOPHIA_DIR/transaction/sx_v.c $SOPHIA_DIR/repository/sy.c $SOPHIA_DIR/environment/se.c $SOPHIA_DIR/environment/se_conf.c $SOPHIA_DIR/environment/se_confcursor.c $SOPHIA_DIR/environment/se_cursor.c $SOPHIA_DIR/environment/se_db.c $SOPHIA_DIR/environment/se_document.c $SOPHIA_DIR/environment/se_execute.c $SOPHIA_DIR/environment/se_o.c $SOPHIA_DIR/environment/se_recover.c $SOPHIA_DIR/environment/se_req.c $SOPHIA_DIR/environment/se_scheduler.c $SOPHIA_DIR/environment/se_tx.c $SOPHIA_DIR/environment/se_view.c $SOPHIA_DIR/environment/se_viewdb.c $SOPHIA_DIR/environment/se_worker.c $SOPHIA_DIR/sophia/sophia.c"

    else

        dnl Check for Sophia header
        PHP_ARG_WITH(sophia-includedir, for Sophia header,
        [ --with-sophia-includedir=DIR Sophia header path], yes)

        if test "$PHP_SOPHIA_INCLUDEDIR" != "no" && test "$PHP_SOPHIA_INCLUDEDIR" != "yes"; then
            if test -r "$PHP_SOPHIA_INCLUDEDIR/sophia.h"; then
                SOPHIA_INCLUDES="$PHP_SOPHIA_INCLUDEDIR"
            else
                AC_MSG_ERROR([Can't find Sophia headers under "$PHP_SOPHIA_INCLUDEDIR"])
            fi
        else
            SEARCH_PATH="/usr/local /usr"
            SEARCH_FOR="/include/sophia.h"
            if test -r $PHP_SOPHIA/$SEARCH_FOR; then
                SOPHIA_INCLUDES="$PHP_SOPHIA/include"
            else
                AC_MSG_CHECKING([for Sophia header files in default path])
                for i in $SEARCH_PATH ; do
                    if test -r $i/$SEARCH_FOR; then
                      SOPHIA_INCLUDES="$i/include"
                      AC_MSG_RESULT(found in $i)
                    fi
                done
            fi
        fi

        if test -z "$SOPHIA_INCLUDES"; then
            AC_MSG_RESULT([not found])
            AC_MSG_ERROR([Can't find Sophia headers])
        fi

        PHP_ADD_INCLUDE($SOPHIA_INCLUDES)

        dnl Check for Sophia library
        PHP_ARG_WITH(sophia-libdir, for Sophia library,
        [ --with-sophia-libdir=DIR Sophia library path], yes)

        LIBNAME_C=sophia
        AC_MSG_CHECKING([for Sophia])
        AC_LANG_SAVE

        save_CFLAGS="$CFLAGS"
        sophia_CFLAGS="-I$SOPHIA_INCLUDES"
        CFLAGS="$save_CFLAGS $sophia_CFLAGS"

        save_LDFLAGS="$LDFLAGS"
        sophia_LDFLAGS="-L$PHP_SOPHIA_LIBDIR -l$LIBNAME_C -lpthread"
        LDFLAGS="$save_LDFLAGS $sophia_LDFLAGS"

        AC_TRY_LINK(
        [
            #include "sophia.h"
        ],[
            void *env = sp_env();
        ],[
            AC_MSG_RESULT(yes)
            SOPHIA_SHARED_LIBADD="$SOPHIA_SHARED_LIBADD -lpthread"
            PHP_ADD_LIBRARY_WITH_PATH($LIBNAME_C, $PHP_SOPHIA_LIBDIR, SOPHIA_SHARED_LIBADD)
            AC_DEFINE(HAVE_SOPHIALIB,1,[ ])
        ],[
            AC_MSG_RESULT([error])
            AC_MSG_ERROR([wrong Sophia lib version or lib not found])
        ])
        CFLAGS="$save_CFLAGS"
        LDFLAGS="$save_LDFLAGS"

        PHP_SUBST(SOPHIA_SHARED_LIBADD)

    fi

    PHP_NEW_EXTENSION(sophia, sophia.c db.c exception.c cursor.c $SOPHIA_SOURCES, $ext_shared)

    ifdef([PHP_INSTALL_HEADERS],
    [
        PHP_INSTALL_HEADERS([ext/sophia/], [php_sophia.h])
    ], [
        PHP_ADD_MAKEFILE_FRAGMENT
    ])
fi

dnl coverage
PHP_ARG_ENABLE(coverage, whether to enable coverage support,
[  --enable-coverage     Enable coverage support], no, no)

if test "$PHP_COVERAGE" != "no"; then
    EXTRA_CFLAGS="--coverage"
    PHP_SUBST(EXTRA_CFLAGS)
fi
