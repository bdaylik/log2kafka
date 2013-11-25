log2kafka
=========


Requirements
------------

* [Kafka C++ Client](https://github.com/adobe-research/libkafka)
* [Avro C++](http://avro.apache.org/docs/current/api/cpp/html/index.html)
* [Apache log4cxx](http://logging.apache.org/log4cxx/)

Build
-----

Building from sources requires cmake 2.6 or later.

To create a release under Unix or Cygwin use:

```bash
  ./cmake_build.sh
```

Check the resulting binary in the ``build/release/src`` folder.


Usage
-----

To get usage help execute:

```bash
  ./log2kafka --help
```

**Apache Log Configuration**

To pipe the log entries under Apache add a new **CustomLog** instruction in the server configuration file. 

Example:

```apache
  CustomLog "|log2kafka -t test_topic -h kafka_broker -p 9092 -s apache-combined -l /etc/log2kafka/log4cxx.properties" combined
```

TODO
----

* Allow partition selection from command line.
* Make the apache log4cxx optional.
* Create installation script.

