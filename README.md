log2kafka
=========

> TODO: Elaborate here

Requirements
------------

* [Boost library](http://www.boost.org) version 1.38 or later. Apart from the header-only libraries of Boost, log2kafka requires filesystem, system, program_options and xpressive libraries.
* [Kafka C Library](https://github.com/edenhill/librdkafka)
* [Avro C++](http://avro.apache.org/docs/current/api/cpp/html/index.html)
* [Apache log4cxx](http://logging.apache.org/log4cxx/)

For Boost and log4cxx dependencies you could use the following installation procedures:

On **Red Hat**:

```bash
sudo yum install boost apr apr-util
```

On **Debian/Ubuntu**:

```bash
sudo apt-get install boost apr apr-util
```

For Kafka and Avro C++ libraries see the next section.


Installation
------------

You can download a distribution package for your platform here:

> TODO: indicate link

Or build the project from sources.

### Build from sources

Building from sources requires cmake 2.6 or later. 

To build under Unix or Cygwin use:

```bash
./cmake_run.sh
```

To create a distribution package use:

```bash
./cmake_run.sh package
```

Depending of your plaform, you could find archives and installation scripts in the `build/release` or `dist` folder.

### Install

* If you use an archive file, this can be uncompressed directly over your filesystem. They have the following folder layout:

```
- etc
  `- log2kafka
- usr
  |- local
  |  |- bin
  |  |- include
  |  |  |- avro
  |  |  `- log4cxx
  |  `- lib
  `- share
     `- doc
        `- log2kafka-<version>
```   
        
* In the case of use an installation script (ej. `log2kafka-1.0.0-Linux.sh`), this will install log2kafka under `<current folder>/log2kafka` and has the same folder layout previously described. To install in another location specify `--prefix=<location>` when running the installation script.

For example, to install under the root of the filesystem, use:

```bash
./log2kafka-1.0.0-Linux.sh --prefix=/
```

When prompted, answer "no" to install in the folder that you has indicated with the prefix argument.

Usage
-----

To get usage help execute:

```bash
./log2kafka --help
```

### Piped Log Configuration

#### Apache

To pipe the log entries under Apache add a new **CustomLog** instruction in the server configuration file. 

Example:

```apache
CustomLog "|log2kafka -t test_topic -h kafka_broker -p 9092 -s apache-combined -l /etc/log2kafka/log4cxx.properties" combined
```

TODO
----

* ~~Allow partition selection from command line~~.
* Make the apache log4cxx optional.
* ~~Create installation script~~.
* Allow read librdkafka configuration options from a properties file.
* Unformat Avro schema metadata header.
* Add unit tests sets

License
-------

Licensed under the Apache Software License 2.0. See [LICENSE](LICENSE) file.
