--TEST--
multipart
--SKIPIF--
--FILE--
<?php
namespace Sophia;

$db = new Db('multipart', array(
    'db.multipart.index.key' => 'string',
    'db.multipart.index' => 'num',
    'db.multipart.index.num' => 'u32',
));

$data = array(
    1 => 'abc',
    2 => 'defg',
    4 => 'hi',
    6 => 'jklmn',
    7 => 'abc',
    8 => 'defg',
    10 => 'hi',
    11 => 'jklmn',
);

echo "*** set ***\n";
foreach ($data as $num => $key) {
    var_dump($db->set(array('key' => $key, 'num' => $num), $key . '-' . $num));
}

echo "*** get ***\n";
var_dump($db->get(array('key' => 'hi', 'num' => 3)));
foreach ($data as $num => $key) {
    var_dump($db->get(array('key' => $key, 'num' => $num)));
}

echo "*** delete ***\n";
var_dump($db->delete(array('key' => 'hi', 'num' => 3)));
var_dump($db->delete(array('key' => 'hi', 'num' => 10)));

echo "*** cursor ***\n";
$cursor = $db->cursor('>=', array('key' => 'defg', 'num' => 8));
foreach ($cursor as $key => $val) {
    var_dump($key);
    var_dump($val);
    var_dump($cursor->keys());
}

?>
--EXPECTF--
*** set ***
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
*** get ***
bool(false)
string(5) "abc-1"
string(6) "defg-2"
string(4) "hi-4"
string(7) "jklmn-6"
string(5) "abc-7"
string(6) "defg-8"
string(5) "hi-10"
string(8) "jklmn-11"
*** delete ***
bool(true)
bool(true)
*** cursor ***
string(4) "defg"
string(6) "defg-8"
array(2) {
  ["key"]=>
  string(4) "defg"
  ["num"]=>
  int(8)
}
string(2) "hi"
string(4) "hi-4"
array(2) {
  ["key"]=>
  string(2) "hi"
  ["num"]=>
  int(4)
}
string(5) "jklmn"
string(7) "jklmn-6"
array(2) {
  ["key"]=>
  string(5) "jklmn"
  ["num"]=>
  int(6)
}
string(5) "jklmn"
string(8) "jklmn-11"
array(2) {
  ["key"]=>
  string(5) "jklmn"
  ["num"]=>
  int(11)
}
