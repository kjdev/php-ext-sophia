# PHP Extension for Sophia

This extension allows Sophia.

Documentation for Sophia can be found at [» http://sphia.org](http://sphia.org).

## Build

Required install Sophia.

``` bash
% phpize
% ./configure
% make
% make install
```

### Build option

* --with-sophia-includedir

  Sophiar include (sophia.h) directory. (ex: /usr/include)

* --with-sophia-libdir

  Sophia library (libsophia.so) directory. (ex: /usr/lib64)

## Configration

sophia.ini:

```
extension=sophia.so

; sophia.path="./storage"
```

## Class synopsis

```
Sophia\Db {
  public __construct( string $db [ , array $options ] )
  public bool set( string|array $key, string $value )
  public mixed get( string|array $key )
  public bool delete( string|array $key )
  public bool close ( void )
  public bool begin( void )
  public bool commit( void )
  public bool rollback( void )
  public Sophia\Cursor cursor( [ string $order, string|array $key ] )
}
```

```
Sophia\Cursor {
  public __construct(Sophia\Db $db [ , string $order, string|array $key ] )
  public bool rewind(void)
  public bool next(void)
  public bool valid(void)
  public string current(void)
  public string key(void)
  public array keys(void)
}
```

## Examples

### basic

```php
namespace Sophia;

$sp = new Db("test");
// $sp = new Db("test", ["sophia.path" => __DIR__ . "/sophia.storage"]);

// set, get, delete
$sp->set("key", "value");
$value = $sp->get("key");
$sp->delete("key");

// iterator
$cursor = $sp->cursor();
// $cursor = $sp->cursor("<");
// $cursor = $sp->cursor(">=", "key");
foreach ($cursor as $key => $value) {
    echo "{$key} => {$value}\n";
}
```

### transaction

```php
namespace Sophia;

$sp = new Db("test_transaction");

// transaction
$sp->begin();
$sp->set("key", "value");
$sp->commit();
// $sp->rollback();
```

### multipart key

```php
namespace Sophia;

$sp = new Db("test_multipart", [
    "db.test_multipart.index.key" => "string",
    "db.test_multipart.index" => "number",
    "db.test_multipart.index.number" => "u32",
]);

// set, get, delete
$sp->set(["key" => "key", "number" => 1], "value-1");
$sp->set(["key" => "key", "number" => 2], "value-2");
$sp->set(["key" => "key", "number" => 3], "value-3");

$value = $sp->get(["key" => "key", "number" => 1]);
/*
output:
  value-1
*/

$sp->delete(["key" => "key", "number" => 2]);

// iterator
$cursor = $sp->cursor(">=", ["key" => "key", "number" => 2]);
foreach ($cursor as $key => $value) {
    echo "{$key} => {$value}\n";
    var_dump($cursor->keys());
}
/*
output:
  key => values-3
  array(2) {
    ["key"]=>
    string(3) "key"
    ["number"]=>
    int(3)
  }
*/
```
