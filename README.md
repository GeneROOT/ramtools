ROOT scripts to convert a SAM file to a RAM (ROOT Alignment/Map) file.

  - To convert a SAM file to a RAM file do:

```bash
  $ root
  root [0] .x makeram.C
  root [1] .q
```

 - To test read a RAM file do:

```bash
    $ root
    root [0] .x ramreader.C
    root [1] .q
```

 - To view a specific region do:

```bash
    $ root
    root [0] .x ramview.C("ramexample.root","chr1:10150-10300")
    root [1] .q
```
