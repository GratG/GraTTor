# GraTTor

BitTorrent client written in C++ using Qt

## Building

Clone using 'git clone' then cd into directory, then build

```
$ mkdir build
$ cmake ..
$ make
```

### Running

After the project is built, run it using ./GraTTor

## Additional notes

    - Only works for single file torrents
    - It is slow, peer connections need to be reworked
    - The program currently does not seed completed files
