# castget

castget is a simple, command-line based RSS enclosure downloader. It is
primarily intended for automatic, unattended downloading of podcasts.

## Installation and configuration

Please see INSTALL for detailed installation instructions.

Dependencies:

  * glib2 (version 2.14 or higher for optional regular expression filters)
  * libcurl
  * id3lib (optional, for ID3 tag support)

See `configure --help` for instructions on how to disable these features should
any of these libraries be unavailable.

You will find a sample configuration file castgetrc.example in the top level
directory of the distribution. You should copy this file to your home directory
as `.castgetrc` and edit it to suit your preferences.

## Usage

For usage instructions see the
[castget(1)](http://mlj.github.io/castget/castget.1.html) and
[castget(5)](http://mlj.github.io/castget/castgetrc.5.html) man pages.

## Development

To rebuild the man pages you will need [ronn](http://rtomayko.github.io/ronn/).
Prebuilt man pages are included in the distribution.

## Bug reports

Please use the [github bug tracker](https://github.com/mlj/castget/issues) to
report bugs.

## License

castget is maintained by Marius L. Jøhndal and is available under the LGPL license.

