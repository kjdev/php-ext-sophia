--TEST--
phpinfo() displays sophia info
--SKIPIF--
--FILE--
<?php

phpinfo();
?>
--EXPECTF--
%a
sophia

Sophia support => enabled
Extension Version => %d.%d.%d
%a
