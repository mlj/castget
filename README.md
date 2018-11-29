# castget

castget is a simple, command-line based RSS enclosure downloader. It is
primarily intended for automatic, unattended downloading of podcasts.

## Installation and configuration

To build and install from a distribution tarball, do the following:

```shell
./configure
make
make install
```

castget depends on

  * glib2 >= 2.30
  * libcurl >= 7.21.6
  * id3lib (optional, for ID3 tag support)

To disable id3lib support, pass `--disable-id3lib` to `configure`;

```shell
./configure --disable-id3lib
```

If building from git, first regenerate the `configure` script using `autoreconf -fi`:

```shell
autoreconf -fi
./configure
make
make install
```

You will also need [ronn](http://rtomayko.github.io/ronn/), which is used to build the man pages. Prebuilt man pages are included in the distribution.

Please see INSTALL for detailed installation instructions.

## Usage

For usage instructions see the
[castget(1)](http://mlj.github.io/castget/castget.1.html) and
[castget(5)](http://mlj.github.io/castget/castgetrc.5.html) man pages.

You will find a sample configuration file castgetrc.example in the top level
directory of the distribution. You should copy this file to your home directory
as `.castgetrc` and edit it to suit your preferences.

## Bug reports

Please use the [github bug tracker](https://github.com/mlj/castget/issues) to
report bugs.

## License

castget is maintained by Marius L. Jøhndal and is available under the LGPL license.

