ROOT scripts to convert a SAM file to a RAM (ROOT Alignment/Map) file.

  - To convert a SAM file to a RAM file do:

    $ root
    root [0] .x makeram.C
    root [1] .q

 - To test read a RAM file do:

    $ root
    root [0] .x ramreader.C
    root [1] .q
