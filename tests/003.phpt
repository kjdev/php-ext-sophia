--TEST--
iterate thought db
--FILE--
<?php
namespace Sophia;

$db = new Db('iterator');

$data = array(
    "First", "Second", "Third", 10, "", "Last"
);

foreach($data as $item) {
    $db->set($item, $item);
}

$cursor = $db->cursor();

echo "*** Loop through ***\n";
for ($cursor->rewind(); $cursor->valid(); $cursor->next()) {
    echo $cursor->key() . " => " . $cursor->current() . "\n";
}

echo "\n*** Seek to give key ***\n";
$cursor = $db->cursor(">=", "Second");
var_dump($cursor->current());

echo "\n*** Seek to a non-exist key will point to nearest next key ***\n";
$cursor = $db->cursor(">=", "11");
var_dump($cursor->current());

echo "\n*** Bound checking ***\n";
$cursor = $db->cursor();
$cursor->rewind();

$cursor->next();
$cursor->next();
$cursor->next();
$cursor->next();
$cursor->next();
$cursor->next();
$cursor->next();
$cursor->next();
$cursor->next();
$cursor->next();
var_dump($cursor->current());
?>
--EXPECTF--
*** Loop through ***
 => 
10 => 10
First => First
Last => Last
Second => Second
Third => Third

*** Seek to give key ***
string(6) "Second"

*** Seek to a non-exist key will point to nearest next key ***
string(5) "First"

*** Bound checking ***
bool(false)
