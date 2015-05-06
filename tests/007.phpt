--TEST--
transaction
--SKIPIF--
--FILE--
<?php
namespace Sophia;

$db = new Db('transaction');

echo "*** transaction start 1 ***\n";
var_dump($db->begin());

var_dump($db->set('key11', 'value11'));
var_dump($db->get('key11'));

var_dump($db->set('key12', 'value12'));
var_dump($db->delete('key12'));

echo "*** transaction commit ***\n";
var_dump($db->commit());
var_dump($db->get('key11'));
var_dump($db->get('key12'));


echo "*** transaction start 2 ***\n";
var_dump($db->begin());

var_dump($db->set('key21', 'value21'));
var_dump($db->get('key21'));

var_dump($db->set('key22', 'value22'));
var_dump($db->delete('key22'));

echo "*** transaction rollack ***\n";
var_dump($db->rollback());
var_dump($db->get('key21'));
var_dump($db->get('key22'));


echo "*** transaction start 3 ***\n";
var_dump($db->begin());

var_dump($db->set('key3', 'value3'));
var_dump($db->get('key3'));

var_dump($db->set('key3', 'value3-a'));
var_dump($db->get('key3'));

echo "*** transaction commit ***\n";
var_dump($db->commit());
var_dump($db->get('key3'));


echo "*** transaction start 4 ***\n";
var_dump($db->begin());

var_dump($db->set('key4', 'value4'));
var_dump($db->get('key4'));

var_dump($db->set('key4', 'value4-a'));
var_dump($db->get('key4'));

echo "*** transaction rollback ***\n";
var_dump($db->rollback());
var_dump($db->get('key4'));


?>
--EXPECTF--
*** transaction start 1 ***
bool(true)
bool(true)
string(7) "value11"
bool(true)
bool(true)
*** transaction commit ***
bool(true)
string(7) "value11"
bool(false)
*** transaction start 2 ***
bool(true)
bool(true)
string(7) "value21"
bool(true)
bool(true)
*** transaction rollack ***
bool(true)
bool(false)
bool(false)
*** transaction start 3 ***
bool(true)
bool(true)
string(6) "value3"
bool(true)
string(8) "value3-a"
*** transaction commit ***
bool(true)
string(8) "value3-a"
*** transaction start 4 ***
bool(true)
bool(true)
string(6) "value4"
bool(true)
string(8) "value4-a"
*** transaction rollback ***
bool(true)
bool(false)
