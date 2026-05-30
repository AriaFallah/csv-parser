# Architecture

`CsvParser` is a small streaming parser. It reads bytes from a `std::istream`,
keeps a fixed-size input buffer, and exposes parsed data through `next_field()`
or the row iterator.

## Main Pieces

```text
+----------------------+     +----------------------+     +----------------------+
| User code            |     | CsvParser            |     | std::istream         |
|----------------------|     |----------------------|     |----------------------|
| next_field()         | --> | parser state machine | --> | file/string/stdin    |
| for (row : parser)   |     | input buffer         |     | borrowed or owned    |
| position()           |     | field buffer         |     |                      |
+----------------------+     +----------------------+     +----------------------+
```

The parser can either borrow a stream or own one:

```text
+----------------------+     +----------------------+
| Borrowed stream      |     | Owned stream         |
|----------------------|     |----------------------|
| CsvParser(istream&)  |     | CsvParser(unique_ptr)|
| caller owns stream   |     | parser owns stream   |
| caller manages life  |     | safe for stored use  |
+----------------------+     +----------------------+
```

`CsvParser::from_file(path)` is a convenience wrapper around the owned-stream
case.

## Data Flow

```text
+--------------+     +--------------+     +--------------+     +--------------+
| input stream | --> | input buffer | --> | next_field() | --> | Field        |
| read()       |     | 128 KiB      |     | CSV states   |     | DATA/ROW/END |
+--------------+     +--------------+     +--------------+     +--------------+
                              |                   |
                              |                   v
                              |            +--------------+
                              +----------> | field buffer |
                                           | std::string  |
                                           +--------------+
```

The row iterator is built on top of `next_field()`:

```text
+--------------+     +--------------+     +----------------------+
| next_field() | --> | iterator     | --> | vector<string> row   |
| field events |     | row builder  |     | reused between rows  |
+--------------+     +--------------+     +----------------------+
```

## Parser States

```text
+----------------+       quote        +-------------------+
| START_OF_FIELD | -----------------> | IN_QUOTED_FIELD   |
+----------------+                    +-------------------+
        |                                      |
        | normal char                          | quote
        v                                      v
+----------------+       delimiter    +-------------------+
| IN_FIELD       | -----------------> | IN_ESCAPED_QUOTE  |
+----------------+                    +-------------------+
        |                                      |
        | newline                              | delimiter/newline
        v                                      v
+----------------+                    +-------------------+
| END_OF_ROW     | -----------------> | START_OF_FIELD    |
+----------------+                    +-------------------+
```

`EMPTY` is the terminal state after CSV end.

## Field Scanning

The state machine still owns the CSV rules. The scanner has a small optimization
for ordinary field text: once it knows it is inside an unquoted or quoted field,
it copies contiguous field characters as a range until the next structural
character.

```text
Unquoted field:

abc,def
^^^
copy chars until comma or row terminator

Quoted field:

"abc,def"
 ^^^^^^^
copy chars until quote
```

That is what the helper names mean:

```text
has_unquoted_field_chars()     more ordinary unquoted bytes are buffered
append_unquoted_field_chars()  append them to the current field
has_quoted_field_chars()       more quoted bytes are buffered
append_quoted_field_chars()    append them to the current field
```

The optimization does not change the public API. It only avoids appending one
character at a time when the next several bytes are known to be field contents.

## Empty Lines

An empty line is a row boundary, not an empty data field.

```text
Input bytes:        \n\n
next_field():       ROW_END
next_field():       ROW_END
next_field():       CSV_END
```

An empty field still returns `DATA`:

```text
Input bytes:        ,
next_field():       DATA("")
next_field():       DATA("")
next_field():       CSV_END
```
