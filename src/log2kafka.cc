/**
 * @file log2kafka.cc
 * @brief utility to serialize and send a web application's log entries to Apache Kafka
 * @author Reinaldo Silva
 * @version 1.0
 * @date 2013
 * @copyright Copyright 2013 Produban. All rights reserved.
 * @copyright Licensed under the Apache License, Version 2.0
 * @copyright http://www.apache.org/licenses/LICENSE-2.0
 */

/*
 * Copyright 2013 Produban
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*! \mainpage Log 2 Kafka Producer
 *
 * \section intro_sec Introduction
 *
 * log2kafka is an utility to serialize and send a web application's log
 * entries to Apache Kafka.
 *
 * Each entry could be sent as:
 *
 * * Raw text, or
 * * An Apache Avro binary serialized message using a record schema of
 * primitive types attributes
 *
 * \section install_sec Installation
 *
 * \subsection step1 Step 1: Opening the box
 *
 * > TODO
 */

#include "log2kafka.hh"

using namespace std;
using namespace log4cxx;
using namespace log4cxx::helpers;

namespace po = boost::program_options;

LoggerPtr logger(Logger::getLogger(BUILD_NAME));

/* Forward function declaration */

inline void validateArguments(const po::variables_map& vm);
inline void debugArguments(const po::variables_map& vm);

/**
 * Main function.
 */
int main(int argc, char** argv) {

    int result = EXIT_SUCCESS;

    po::options_description description("Allowed options");

    description.add_options()
    ("help,?", "produce help message")
    ("client,c", po::value<std::string>()->default_value(Constants::DEFAULT_CLIENT_ID),
            "producer client name/id")
    ("host,h", po::value<std::string>(), "broker hostname/ip")
    ("port,p", po::value<int>(), "broker port number")
    ("topic,t", po::value<std::string>(), "target topic in the form <topic_name>[:<partition>]")
    ("schema,s", po::value<std::string>(),
            "(optional) avro definitition file to use for serialization - if omitted the raw entry will be sent")
    ("key,k", po::value<string>()->default_value(Constants::DEFAULT_MESSAGE_KEY), "kafka message key to use")
    ("codec,z", po::value<string>(), "(optional) Compression codec: gzip|snappy")
    ("log-config,l", po::value<std::string>(), "(optional) log4cxx configuration file path")
    ("kafka-config,k", po::value<string>(),
            "(optional) Additional librdkafka configuration options file path")
    ("message,m", po::value<std::string>(),
            "(optional) message to send - if not indicated then standard input is used")
    ("version", "display version number");

    po::variables_map vm;

    /* Process arguments */

    try {
        po::store(po::parse_command_line(argc, argv, description), vm);

        if (vm.count("help")) {
            cout << description;
            return result;
        }
        else if (vm.count("version")) {
            cout << BUILD_NAME << " " << log2kafka_VERSION << std::endl;
            return result;
        }

        po::notify(vm);

        validateArguments(vm);
    }
    catch (exception& e) {
        cerr << "ERROR: " << e.what() << endl;
        return EXIT_FAILURE;
    }

    /* Configure logger */

    if (vm.count("log-config")) {
        File logconfig(vm["log-config"].as<string>());
        PropertyConfigurator::configure(logconfig);
    }

    LoggerPtr rootLooger = Logger::getRootLogger();

    if (rootLooger->getAllAppenders().size() == 0) {

        // No appenders configured. Default to console with WARN level

        BasicConfigurator::configure();
        rootLooger->setLevel(Level::getWarn());
    }

    if (LOG4CXX_UNLIKELY(logger->isDebugEnabled())) {
        debugArguments(vm);
    }

    /* Socket hangups are gracefully handled in librdkafka on socket error
     * without the use of signals, so SIGPIPE should be ignored by
     * the calling program. */
    signal(SIGPIPE, SIG_IGN );

    /* Select action course route */

    try {
        /* Prepare client connection proxy */

        auto proxy = unique_ptr<ClientFacade>(new ClientFacade());

        proxy->clientId(vm["client"].as<string>());
        proxy->messageKey(vm["key"].as<string>());
        proxy->host(vm["host"].as<string>());
        proxy->port(vm["port"].as<int>());
        proxy->topic(vm["topic"].as<string>());

        if (vm.count("codec")) {
            proxy->codec(vm["codec"].as<string>());
        }

        if (vm.count("schema")) {
            LOG4CXX_DEBUG(logger, "Schema defined. Using AVRO serialization mode");
            proxy->serializer(vm["schema"].as<string>());
        }

        proxy->connect();

        /* Retrieve message to serialize */

        string entry;

        if (vm.count("message")) {
            entry = vm["message"].as<string>();
            proxy->sendMessage(entry);
        }
        else { // read from standard input
            LOG4CXX_DEBUG(logger, "Read from standard input");

            /* Read a buffer's worth of log file data, exiting on errors */

            string line;

            for (;;) {
                getline(cin, line);

                if (cin.fail()) break;

                proxy->sendMessage(line);
            }
        }
    }
    catch (exception& e) {
        cerr << "Unexpected exception: " << e.what() << endl;
        LOG4CXX_ERROR(logger, "Unexpected exception: " << e.what());
        result = EXIT_FAILURE;
    }
    catch (...) {
        cerr << "Unexpected exception." << endl;
        LOG4CXX_ERROR(logger, "Unexpected exception ");
        result = EXIT_FAILURE;
    }

    return result;
}

inline void validateArguments(const po::variables_map& vm) {

    if (!vm.count("host")) {
        throw invalid_argument("'host' argument was not set.");
    }

    if (!vm.count("port")) {
        throw invalid_argument("'port' argument was not set.");
    }

    if (!vm.count("topic")) {
        throw invalid_argument("'topic' argument was not set.");
    }
}

inline void debugArguments(const po::variables_map& vm) {
    LOG4CXX_DEBUG(logger, "Arguments:"
            << "\n\tclient: " << vm["client"].as<string>()
            << "\n\tkey: " << vm["key"].as<string>()
            << "\n\thost: " << vm["host"].as<string>()
            << "\n\tport: " << vm["port"].as<int>()
            << "\n\ttopic: " << vm["topic"].as<string>()
            << "\n\tschema: " << (vm.count("schema") ? vm["schema"].as<string>() : "")
            << "\n\tcodec: " << (vm.count("codec") ? vm["codec"].as<string>() : "")
            << "\n\tlog-config: " << (vm.count("log-config") ? vm["log-config"].as<string>() : "")
            << "\n\tkafka-config: " << (vm.count("kafka-config") ? vm["kafka-config"].as<string>() : ""));
}
