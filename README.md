# castget

castget is a simple, command-line based RSS enclosure downloader. It is
primarily intended for automatic, unattended downloading of podcasts.

## Packages

Many distributions have packages for castget:

[![Packaging status](https://repology.org/badge/vertical-allrepos/castget.svg?header=castget)](https://repology.org/project/castget/versions)  

## Building from source

| Branch | Linux (clang) | Linux (gcc) | macOS |
|--------|---------------|-------------|-------|
|master|[![Linux (clang) Build Status](https://travis-matrix-badges.herokuapp.com/repos/mlj/castget/branches/master/2)](https://travis-ci.org/mlj/castget)|[![Linux (gcc) Build Status](https://travis-matrix-badges.herokuapp.com/repos/mlj/castget/branches/master/3)](https://travis-ci.org/mlj/castget)|[![macOS Build Status](https://travis-matrix-badges.herokuapp.com/repos/mlj/castget/branches/master/1)](https://travis-ci.org/mlj/castget)|

To build from a distribution tarball, do the following:

```shell
./configure
make
make install
```

castget depends on

  * glib2 >= 2.30
  * libcurl >= 7.21.6
  * taglib (optional)

If building on macOS, you can use Homebrew to install the dependencies:

```shell
brew install glib taglib
```

To disable taglib support, pass `--without-taglib` to `configure`;

```shell
./configure --without-taglib
```

To build from git, clone the master branch

```shell
git clone https://github.com/mlj/castget.git
```

then rebuild the autoconf scripts

```shell
autoreconf -fi
./configure
make
make install
```

You will also need [ronn](http://rtomayko.github.io/ronn/), which is used to
build the man pages. Prebuilt man pages are included in the distribution.

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
