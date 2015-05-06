--TEST--
iterate thought db by foreach
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

echo "*** Loop through in foreach style ***\n";
foreach ($cursor as $key => $vaue) {
    echo $cursor->key() . " => " . $cursor->current() . "\n";
}
?>
--EXPECTF--
*** Loop through in foreach style ***
 => 
10 => 10
First => First
Last => Last
Second => Second
Third => Third
