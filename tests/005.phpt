--TEST--
db close
--SKIPIF--
--FILE--
<?php
namespace Sophia;

$db = new Db('close');

$db->set('key', 'value');

$curosr = $db->cursor();

$db->close();

try {
    $db->set('new-key', 'value');
} catch(Exception $e) {
    echo $e->getMessage() . "\n";
}

try {
    $curosr->next();
} catch(Exception $e) {
    echo $e->getMessage() . "\n";
}
?>
--EXPECTF--
Can not operate on closed database
Can not iterate on closed database
