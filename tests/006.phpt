--TEST--
different iterators should not affect each other
--SKIPIF--
--FILE--
<?php
namespace Sophia;

$db = new Db('iterator_should_not_affect_eachother');

foreach(array(1, 2, 3, 4) as $k) {
    $db->set($k, $k);
}

$curosr1 = $db->cursor();
$curosr2 = $db->cursor();

foreach($curosr1 as $k => $v) {
    echo "$k => $v\n";
}

$curosr1->rewind();
var_dump($curosr1->next());
var_dump($curosr1->next());
var_dump($curosr1->current());
var_dump($curosr2->current());
?>
--EXPECTF--
1 => 1
2 => 2
3 => 3
4 => 4
NULL
NULL
string(1) "3"
string(1) "1"
