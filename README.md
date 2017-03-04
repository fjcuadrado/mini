# mini

Mini is a small C library to parse INI files.

## Installation

To install `mini` you must do:

```shell
$ autoreconf --install
$ ./configure
$ make
$ make install
```

If you want to run `mini` unit tests you can do:

```shell
$ make check
```

## Usage

```c
#include <mini.h>

int
main(int argc, char *argv[])
{
    char *ini_file = "/path/to/example.ini";
    MiniFile *mini_file;

    mini_file = mini_parse_file(ini_file);
    if (mini_file == NULL) {
        return -1;
    }

    /* Manage INI file data using mini functions:
    *  mini_get_number_of_sections
    *  mini_get_number_of_keys
    *  mini_get_section
    *  mini_get_key
    *  mini_get_value
    */

    /* Don't forget to free the data allocated by mini! */
    mini_free(mini_file);

    return 0;
}
```
