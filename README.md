
# blkprobe

Probe read/write operations of a file.


## Setup

Install libfuse3-dev.

```
# apt-get install libfuse3-dev
```

And make this application.

```
$ make
```


## Init

Create a large file (for example 512MB ~ 1GB).

```
$ dd if=/dev/zero of=large_file bs=1048576 count=512
```

Launch `blkview` that is the viewer application of read/write operations.
It is easy to launch this application from IntelliJ IDEA.

Refer: https://github.com/katsuster/blkview

Please compile source (*.java) files and launch the application by `java` command if you want to launch manually.

```
$ java net.katsuster.blkview.BlocksViewerMain -classpath ./
```

Launch `blkprobe` using large_file.
And connect to `localhost` that is a hostname or an IP address of a machine running the `blkview` application.
This application shows only a `image` file in `fuse_directory` and watching read/write operaions.

```
# mkdir fuse_directory
# blkprobe large_file localhost -f fuse_directory

# ls fuse_directory/
image
```

Setup loopback device to use `image` file.

```
# losetup /dev/loop0 fuse_directory/image
```

Now, you can see read/write operations for this loopback device.

```
# mkfs.ext2 /dev/loop0

(... blkview shows read/write operations ...)
```

Enjoy!


## Finish

Remove loopback device.

```
losetup -d /dev/loop0
```

And stop the `blkprobe` application by Ctrl+C.
