This is a summary of changes between each release. See the commit log on
https://github.com/mlj/castget/commits/master for a details.

Unreleased development version:

  * Add support for filename patterns (configuration option `filename`)
  * Refuse to overwrite existing downloads unless explicitly asked to resume a
    download
  * Behaviour change: The `-f`/`--filter` option and the filter configuration
    options now match the URLs of enclosures instead of the filenames castget
    infers for them
  * Require glib >= 2.30 and make regular expression support mandatory
  * Drop autogen.sh and require autoreconf when building from git
  * Enable libcurl's compression support (PR #28)

Version 1.2.4 (2017/05/17):

  * Fix build issues involving man pages (issue #20)

Version 1.2.3 (2017/05/16):

  * Fix lack of support for https (issue #10, issue #14, PR #15)
  * Fix broken handling of COLUMNS environment variable etc. (PR #16)
  * Improve error reporting when enclosure download fails
  * Check sanity of reported length of enclosures in feeds
  * Fix MacOS build
  * Replace ChangeLog and NEWS files with CHANGES file

Version 1.2.2 (2016/01/26):

  * Fix width of the progress bar.

Version 1.2.1 (2016/01/04):

  * Fix broken enclosure filenames.

Version 1.2.0 (2013/07/10):

  * Add command line option -p for an optional progress bar.
  * Add command line option --debug for optional connection debugging.
  * Use glib's option parsing.
  * Fix incorrect memory allocation when enclosure download fails.
  * Document proxy configuration.

Version 1.1.0 (2010/02/20):

  * New command line option --rcfile for specifying an alternate location for
  the configuration file.

  * New command line option --filter and configuration key filter for
  restricting operations to enclosures matching a regular
  expression. This feature requires glib >= 2.14 and can be disabled
  by supplying --disable-gregex to configure.

  * Distribution files are compressed with bzip2.

Version 1.0.1 (2007/11/14):

  * castget now identifies itself using the HTTP User-Agent header.

See https://github.com/mlj/castget/blob/4ac9ebd27b35838ae102ff50cf973645614d314e/ChangeLog
for changes between version 0.9.0 (2005/11/14) and version 1.0.0 (2007/09/20).
