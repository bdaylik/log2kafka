log2kafka
=========

log2kafka is an utility to serialize and send a web application's log entries to Apache Kafka.

Each entry could be sent as:

* Raw text, or
* An Apache Avro binary serialized message using a record schema of primitive types attributes

Requirements
------------

* [Boost library](http://www.boost.org) version 1.38 or later. Apart from the header-only libraries of Boost, log2kafka requires filesystem, system, program_options and xpressive libraries.
* [Kafka C Library](https://github.com/edenhill/librdkafka)
* [Avro C++](http://avro.apache.org/docs/current/api/cpp/html/index.html) version 1.7.5. You can download this version at http://archive.apache.org/dist/avro/avro-1.7.5/cpp/ 
* [Apache log4cxx](http://logging.apache.org/log4cxx/) [optional]

For Boost and log4cxx dependencies you could use the following installation procedures:

On **Red Hat**:

```bash
sudo yum install boost apr apr-util
```

On **Debian/Ubuntu**:

```bash
sudo apt-get install boost liblog4cxx10-dev
```

For Kafka and Avro C++ libraries see the next section.


Installation
------------

You can download a distribution package for Linux here:

  https://github.com/Produban/log2kafka/releases/latest

Or build the project from sources.

### Build from sources
Before building you should have the required libraries.

####Steps for installing required libraries on Debian/Ubuntu
##### Install build requirements
Make sure you have cmake 2.6 or later, boost 1.5.4 or later and log4cxx
```
sudo apt-get install build-essential cmake libboost-all-dev liblog4cxx10-dev
```
##### Install Kafka C library
```
wget "https://github.com/edenhill/librdkafka/archive/0.8.3.tar.gz"
tar -xzf 0.8.3.tar.gz
cd  librdkafka-0.8.3
make
sudo make install
cd ..
```
##### Install Avro C++ (1.7.5 is required)
```
wget "http://archive.apache.org/dist/avro/avro-1.7.5/cpp/avro-cpp-1.7.5.tar.gz"
tar -xzf avro-cpp-1.7.5.tar.gz
cd avro-cpp-1.7.5
./build.sh test
sudo ./build.sh install
cd ..
```
That's all for the required libraries.

You may now wish to adjust the following CMAKE options:

* USE_LOG4CXX - Use apache log4cxx library. Default: OFF
* KAFKA_LINK_STATIC - For static linking of kafka library. Default: OFF
* AVRO_LINK_STATIC - For static linking of avro library. Default: OFF
* BUILD_DOC - Create and install the API documentation (requires Doxygen). Default: OFF

To do so, execute:

```bash
export CMAKE_OPTIONS="-DBUILD_DOC:BOOL=ON -DKAFKA_LINK_STATIC:BOOL=ON -DAVRO_LINK_STATIC:BOOL=ON -DUSE_LOG4CXX:BOOL=ON"
```

being the above the same combination used for the pre-built installation script generation. You can of course change this to your desired combination of options. 

Then, to build under *nix or Cygwin use:

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
  |  `- bin
  `- share
     `- doc
        `- log2kafka-<version>
```   
        
* In the case you use an installation script (ej. `log2kafka-<version>-Linux.sh`), this will install log2kafka under `<current folder>/log2kafka` and has the same folder layout previously described. To install in another location specify `--prefix=<location>` when running the installation script.

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

Where, by default all file arguments are relative to the `/etc/log2kafka` if a relative path is indicated.

### Avro Schema Specification

The Avro json schema and corresponding mapping rule is a simple text file composed of a **regular expression pattern** and a **avro schema** separated by a `//--AVRO--` text marker. Several examples can be found at  [/etc/log2kafka](./src/conf).

The following example correspond to the standard **combined** format of the Apache Web Server.

```
pattern : (\d+.\d+.\d+.\d+)\s+([\-\w]+)\s+([\-\w]+)\s+\[(\d+\/\S+\/\d+:\d+:\d+:\d+\s+[-+]{0,1}\d+)\]\s+\"(.*?)\s+HTTP\/\d+\.\d+\"\s+(\d+)\s+(\-|\d+)\s+\"(.*?)\"\s+\"(.*?)\"
//--AVRO--
{
    "namespace": "openbus",
    "type": "record",
    "name": "ApacheCombined",
    "doc": "Apache log entry (combined format)",
    "fields": [
        {"name": "host", "type": "string"},
        {"name": "log", "type": "string"},
        {"name": "user", "type": "string"},
        {"name": "datetime", "type": "string"},
        {"name": "request", "type": "string"},
        {"name": "status", "type": "string"},
        {"name": "size", "type": "string"},
        {"name": "referer", "type": "string"},
        {"name": "agent", "type": "string"}
    ]
}
```

Here, the regular expression groups defined in the **pattern** must match the schema attributes in secuential order. Any failure in the pattern interpretation or in the attribute matching will make log2kafka fallback to sending messages in plain text (as received).

You can also use other Avro primitive types for field specification. For example, in the preceding schema definition the `size` attribute may be declared as `int` or `long`. Again, a failure to validate the schema or the data according to the definition, will cause log2kafka falling back to plain text sending.

Once defined, you can use the schema configuration file with the `--schema` (also `-s`) argument.

Example:

```bash
log2kafka -b kafka_broker:9092 -t test_topic -s apache-combined.conf -f config.ini
```

### INI File Configuration

You can especify execution options from a INI-style configuration file, to do this indicate it using the `--config` command line argument (also `-f`).

Example:

```bash
log2kafka -f config.ini
```
or even:

```bash
log2kafka -b kafka_broker:9092 -t test_topic -s apache-combined.conf -f config.ini
```

If an option is specified in both places, command line and configuration file, those provided from command line take precedence.

The file [/etc/log2kafka/config-sample.ini](./src/conf/config-sample.ini) is provided as example.

### Piped Log Configuration

#### Apache

To pipe the log entries under Apache add a new **CustomLog** instruction in the server configuration file. 

Example:

```apache
CustomLog "|log2kafka -b kafka_broker:9092 -t test_topic -s apache-combined.conf -l log4cxx.properties" combined
```

### Debugging

If your installation was compiled with log4cxx, then configure the appropiate logging level in the file indicated with the argument `--log-config`. The file [/etc/log2kafka/log4cxx-sample.properties](./src/conf/log4cxx-sample.properties) is provided as example.

Otherwise, the following rules apply:

* Always send **ERROR** and **FATAL** messages to `stderr`
* If `--verbose` argument is used, then:
  * Send **INFO** and **WARN** messages to `stdout`
  * Also, if compiled without **NDEBUG** (which is the default), then send **DEBUG** and **TRACE** to `stdout`. Conversely, those levels are ignored if **NDEBUG** was used in the compilation.
   
Additionally, debug levels specific to the kafka libray can be indicated using the `--kafka.debug` argument.

TODO
----

* ~~Make the apache log4cxx optional.~~
* ~~Allow read librdkafka configuration options from a properties file.~~
* ~~Unformat Avro schema metadata header.~~
* Make default config path relative to `/<installation_folder>/etc/log2kafka` instead of `/etc/log2kafka`
* Add unit tests sets

License
-------

Licensed under the Apache Software License 2.0. See [LICENSE](LICENSE) file.
