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
  public bool set( string $key, string $value )
  public mixed get( string $key )
  public bool delete( string $key )
  public bool begin( void )
  public bool commit( void )
  public bool rollback( void )
  public Sophia\Cursor cursor( [ string $order, string $key ] )
}
```

```
Sophia\Cursor {
  public __construct(Sophia\Db $db [ , string $order, string $key ] )
  public bool rewind(void)
  public bool next(void)
  public bool valid(void)
  public string current(void)
  public string key(void)
}
```

## Examples

```php
namespace Sophia;

$sp = new Db("test");
// $sp = new Db("test", ["sophia.path" => __DIR__ . "/sophia.storage"]);

// set, get, delete
$sp->set("key", "value");
$value = $sp->get("key");
$sp->delete("key");

// transaction
$sp->begin();
$sp->set("key", "value");
$sp->commit();
// $sp->rollback();

// iterator
$cursor = $sp->cursor();
// $cursor = $sp->cursor("<");
// $cursor = $sp->cursor(">=", "key");
foreach ($cursor as $key => $value) {
    echo "{$key} => {$value}\n";
}
```
