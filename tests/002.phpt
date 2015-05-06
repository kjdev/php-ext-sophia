--TEST--
storage directory
--INI--
open_basedir=.
--SKIPIF--
--FILE--
<?php
namespace Sophia;

$path = getcwd() . '/../sophia_storage.test-db';

$db = new Db('test', array('sophia.path' => $path));
$db = new Db('test', array('log.path' => $path . '.log'));

?>
--EXPECTF--

Warning: Sophia\Db::__construct(): open_basedir restriction in effect. File(%s) is not within the allowed path(s): (%s) in %s on line %d

Warning: Sophia\Db::__construct(): open_basedir restriction in effect. File(%s) is not within the allowed path(s): (%s) in %s on line %d
