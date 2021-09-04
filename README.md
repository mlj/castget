# castget

castget is a simple, command-line based RSS enclosure downloader. It is
primarily intended for automatic, unattended downloading of podcasts.

## Packages

Many distributions have packages for castget:

[![Packaging status](https://repology.org/badge/vertical-allrepos/castget.svg?header=castget)](https://repology.org/project/castget/versions)  

## Building

castget depends on

  * glib2 >= 2.30
  * libcurl >= 7.21.6
  * taglib (optional)

If building on macOS, you can use Homebrew to install the dependencies:

```shell
brew install glib taglib
```

On Ubuntu Xenial, you need the following:

  * libcurl4-gnutls-dev
  * libtagc0-dev (optional)

On Alpine, you need the following:

  * libxml2-dev
  * glib-dev
  * curl-dev
  * taglib-dev (optional)

On Debian 9 “stretch”, you need the following:

  * pkg-config
  * libglib2.0-dev
  * libxml2-dev
  * libcurl3-dev
  * libtagc0-dev (optional)

### Building from source

To build from a distribution tarball, do the following:

```shell
./configure
make
make install
```

To disable taglib support, pass `--without-taglib` to `configure`;

```shell
./configure --without-taglib
```

### Building from git

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

### Building from Dockerfile

Build docker image

```
docker build -t castget .
```

Create a file `.castgetrc` as shown on [Usage](Usage) and run container.

```
docker run -v $(pwd):/castget --rm -it castget
```

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
