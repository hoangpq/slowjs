# slowjs

A standalone (just libc) JavaScript interpreter.

### Dependencies

* git
* gpp/clang
* cmake
* make

### Example

```bash
$ cat examples/plus.js
function main() {
  return 1 + 3;
}
$ ./bin/slowjs examples/plus.js
4.000000
```

### Build

```bash
$ mkdir build
$ cd build
$ cmake ..
```

### Test

```bash
$ mkdir test
$ cd test
$ ctest ..
```
