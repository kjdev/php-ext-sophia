--TEST--
basic: get(), set(), delete()
--SKIPIF--
--FILE--
<?php
namespace Sophia;

$db = new Db('basic');

var_dump($db->set('key', 'value'));
var_dump($db->get('key'));
var_dump($db->get('non-exists-key'));
var_dump($db->set('name', 'reeze'));
var_dump($db->get('name'));
var_dump($db->delete('name'));
var_dump($db->get('name'));
?>
==DONE==
--EXPECTF--
bool(true)
string(5) "value"
bool(false)
bool(true)
string(5) "reeze"
bool(true)
bool(false)
==DONE==
