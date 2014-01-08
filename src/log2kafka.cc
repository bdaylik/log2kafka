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

#include "ClientFacade.hh"

#ifdef _LOG2KAFKA_USE_LOG4CXX_
using namespace log4cxx;
using namespace log4cxx::helpers;

LoggerPtr logger(Logger::getLogger(BUILD_NAME));
#endif

namespace po = boost::program_options;
using namespace boost::filesystem;
using namespace std;

/* Forward function declaration */

void parseArguments(int argc, char** argv, po::variables_map& vm);
inline void validateArguments(const po::variables_map& vm);
inline void debugArguments(const po::variables_map& vm);

/**
 * Main function.
 */
int main(int argc, char** argv) {

    int result = EXIT_SUCCESS;
    po::variables_map vm;

    /* Process arguments */

    try {
        parseArguments(argc, argv, vm);
    }
    catch (exception& e) {
        cerr << "ERROR: " << e.what() << endl;
        return EXIT_FAILURE;
    }

    /* Configure logger */

#ifdef _LOG2KAFKA_USE_LOG4CXX_
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

    Constants::IS_TRACE_ENABLED = LOG4CXX_UNLIKELY(logger->isTraceEnabled());
    Constants::IS_DEBUG_ENABLED = LOG4CXX_UNLIKELY(logger->isDebugEnabled());
#else
    Constants::LOG_VERBOSITY = (vm.count("verbose") != 0);
    Constants::IS_TRACE_ENABLED = Constants::LOG_VERBOSITY;
    Constants::IS_DEBUG_ENABLED = Constants::LOG_VERBOSITY;
#endif

    debugArguments(vm);

    /* Socket hangups are gracefully handled in librdkafka on socket error
     * without the use of signals, so SIGPIPE should be ignored by
     * the calling program. */
    signal(SIGPIPE, SIG_IGN);

    /* Select action course route */

    try {
        /* Prepare client connection proxy */

        auto proxy = unique_ptr<ClientFacade>(new ClientFacade());
        proxy->configure(vm);

        /* Retrieve message to serialize */

        string entry;

        if (vm.count("message")) {
            entry = vm["message"].as<string>();
            proxy->sendMessage(entry);
        }
        else { // read from standard input
            LOG_DEBUG("Read from standard input");

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
        LOG_ERROR("Unexpected exception: " << e.what());
        result = EXIT_FAILURE;
    }
    catch (...) {
        LOG_ERROR("Unexpected exception ");
        result = EXIT_FAILURE;
    }

    return result;
}

void parseArguments(int argc, char** argv, po::variables_map& vm) {

    po::options_description generic("Generic options");
    po::options_description avroOptions("Avro options");
    po::options_description kafkaOptions("Kafka options");

    /* General options */

    generic.add_options()
    ("version", "display version number")
    ("help,?", "produce help message")
    ("config,f", po::value<std::string>(), "client configuration file path")
    #ifdef _LOG2KAFKA_USE_LOG4CXX_
    ("log-config,l", po::value<std::string>(), "log4cxx configuration file path")
#else
    ("verbose", "increase verbosity")
    #endif
    ("message,m", po::value<std::string>(),
        "message to send - if not indicated then standard input is used");

    /* Avro options */

    avroOptions.add_options()
    ("schema,s", po::value<std::string>(),
        "Avro definitition file to use for serialization - if omitted the raw entry will be sent");

    /* Kafka options */

    kafkaOptions.add_options()
    ("kafka.client.id,c", po::value<std::string>()->default_value(Constants::DEFAULT_CLIENT_ID),
        "producer client name/id")
    ("kafka.metadata.broker.list,b", po::value<std::string>(),
        "A comma separated list of brokers:\n <host>[:<port>][,...]")
    ("kafka.topic,t", po::value<std::string>(), "target topic: <topic_name>[:<partition>]")
    ("kafka_topic.request.required.acks", po::value<int>())
    ("kafka_topic.request.timeout.ms", po::value<int>())
    ("kafka_topic.message.timeout.ms", po::value<int>())
    ("kafka.key,k", po::value<string>(), "kafka message key to use")
    ("kafka.codec,z", po::value<std::string>(), "Compression codec to use: gzip|snappy")
    ("kafka.message.max.bytes", po::value<int>())
    ("kafka.metadata.request.timeout.ms", po::value<int>())
    ("kafka.topic.metadata.refresh.interval.ms", po::value<int>())
    ("kafka.topic.metadata.refresh.fast.cnt", po::value<int>())
    ("kafka.topic.metadata.refresh.fast.interval.ms", po::value<int>())
    ("kafka.socket.timeout.ms", po::value<int>())
    ("kafka.socket.send.buffer.bytes", po::value<int>())
    ("kafka.socket.receive.buffer.bytes", po::value<int>())
    ("kafka.broker.address.ttl", po::value<int>())
    ("kafka.statistics.interval.ms", po::value<int>())
    ("kafka.queued.min.messages", po::value<int>())
    ("kafka.fetch.wait.max.ms", po::value<int>())
    ("kafka.fetch.min.bytes", po::value<int>())
    ("kafka.fetch.error.backoff.ms", po::value<int>())
    ("kafka.queue.buffering.max.messages", po::value<int>())
    ("kafka.queue.buffering.max.ms", po::value<int>())
    ("kafka.message.send.max.retries", po::value<int>())
    ("kafka.retry.backoff.ms", po::value<int>())
    ("kafka.batch.num.messages", po::value<int>())
    ("kafka.request.required.acks", po::value<int>())
    ("kafka.request.timeout.ms", po::value<int>())
    ("kafka.message.timeout.ms", po::value<int>())
    ("kafka.debug", po::value<std::string>(),
        "A comma-separated list of debug contexts to enable: all,generic,broker,topic,metadata,producer,queue,msg")
        ;

    po::options_description cmdline_options;
    cmdline_options.add(generic).add(avroOptions).add(kafkaOptions);

    /*  Parse command line */

    po::store(po::parse_command_line(argc, argv, cmdline_options), vm);

    if (vm.count("help")) {
        cout << cmdline_options;
        exit(EXIT_SUCCESS);
    }
    else if (vm.count("version")) {
        cout << BUILD_NAME << " " << log2kafka_VERSION << endl;
        exit(EXIT_SUCCESS);
    }

    /* Parse config file */

    if (vm.count("config")) {
        path configFilePath(vm["config"].as<string>());
        path configPath(Constants::DEFAULT_CONFIG_PATH);

        if (!configFilePath.is_complete()) {
            configFilePath = configPath / configFilePath;
        }

        if (exists(configFilePath)) {
            ifstream configFile(configFilePath.string());

            LOG_DEBUG("Reading additional options from: " << configFilePath);
            po::store(po::parse_config_file(configFile, kafkaOptions), vm);
        }
        else {
            LOG_WARN("The indicated configuration file '" << configFilePath << "' does not exist");
        }
    }

    /* Validate arguments and prepare options map */

    validateArguments(vm);
    po::notify(vm);
}

inline void validateArguments(const po::variables_map& vm) {

    if (!vm.count("kafka.metadata.broker.list")) {
        throw invalid_argument("'kafka.metadata.broker.list (-b)' argument was not set.");
    }

    if (!vm.count("kafka.topic")) {
        throw invalid_argument("'kafka.topic (-t)' argument was not set.");
    }
}

inline void debugArguments(const po::variables_map& vm) {

    if (Constants::IS_DEBUG_ENABLED) {
        ostringstream buffer("Arguments:");

        for (po::variables_map::const_iterator it = vm.begin(); it != vm.end(); ++it) {
            buffer << "\n\t" << it->first << ": ";

            if (typeid(int) == it->second.value().type()) {
                buffer << it->second.as<int>();
            }
            else {
                buffer << it->second.as<string>();
            }
        }

        LOG_DEBUG(buffer.str());
    }
}
