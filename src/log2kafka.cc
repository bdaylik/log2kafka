/**
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

#include "log2kafka.hh"

using namespace std;
using namespace log4cxx;
using namespace log4cxx::helpers;

namespace po = boost::program_options;
namespace kafka = LibKafka;

LoggerPtr logger(Logger::getLogger(BUILD_NAME));

/* Forward function declaration */

void sendMessage(const po::variables_map& vm, const string& entry);
inline void debugArguments(const po::variables_map& vm);

/**
 * Main function.
 */
int main(int argc, char** argv) {

    int result = EXIT_SUCCESS;

    po::options_description description("Allowed options");
    description.add_options()
    ("help,?", "produce help message")
    ("version", "display version number")
    ("client,c", po::value<std::string>()->default_value(Constants::DEFAULT_CLIENT_ID),
            "producer client name/id")
    ("host,h", po::value<std::string>()->default_value(Constants::DEFAULT_HOST), "hostname/ip")
    ("port,p", po::value<std::size_t>()->default_value(Constants::DEFAULT_PORT), "connection port")
    ("log,l", po::value<std::string>(), "log4cxx configuration file path")
    ("schema,s", po::value<std::string>(),
            "avro schema name to use for serialization - if not indicated then the raw entry will be sent")
    ("topic,t", po::value<std::string>()->default_value(""), "kafka topic")
    ("schema-path", po::value<std::string>()->default_value(Constants::DEFAULT_SCHEMA_PATH),
            "path to look for schema definitions")
    ("message,m", po::value<std::string>(), "message to send - if not indicated then standard input is used");

    po::variables_map vm;

    /* Process arguments */

    try {
        po::store(po::parse_command_line(argc, argv, description), vm);
        po::notify(vm);
    }
    catch (exception& e) {
        cerr << "ERROR: " << e.what() << endl;
        return EXIT_FAILURE;
    }

    if (vm.count("log")) {
        File logconfig(vm["log"].as<string>());
        PropertyConfigurator::configure(logconfig);
    }
    else {
        BasicConfigurator::configure();
    }

    if (LOG4CXX_UNLIKELY(logger->isDebugEnabled())) {
        debugArguments(vm);
    }

    /* Select action course route */

    if (vm.count("help")) {
        cout << description;
    }
    else if (vm.count("version")) {
        cout << BUILD_NAME << " " << log2kafka_VERSION << std::endl;
    }
    else {
        try {
            string entry;

            /* Retrieve message to serialize */

            if (vm.count("message")) {
                entry = vm["message"].as<string>();
                sendMessage(vm, entry);
            }
            else { // read from standard input
                LOG4CXX_DEBUG(logger, "Read from standard input");

                /* Read a buffer's worth of log file data, exiting on errors
                 * or end of file.
                 */

                string line;

                for (;;) {
                    getline(cin, line);

                    if (cin.fail()) break;

                    sendMessage(vm, line);
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
    }

    return result;
}

void sendMessage(const po::variables_map& vm, const string& entry) {

    LOG4CXX_DEBUG(logger, "Message: " << entry);

    if (entry.length() == 0) {
        LOG4CXX_WARN(logger, "Empty message entry discarded");
    }
    else {

        ClientProxy proxy(vm["host"].as<string>(), vm["port"].as<size_t>(), vm["topic"].as<string>());
        string message(entry);

        if (vm["schema"].as<string>() == "") { // No schema. Send raw entry.
            LOG4CXX_DEBUG(logger, "No schema defined. Using raw mode");
        }
        else { // Serialize entry
            LOG4CXX_DEBUG(logger, "Schema defined. Using serialization mode");

            Serializer serializer(
                    vm["schema-path"].as<string>() + "/" + vm["schema"].as<string>() + ".json");

            try {
                message = serializer.serialize(entry);
            }
            catch (exception& e) {
                LOG4CXX_ERROR(logger, "Using raw mode due to unexpected exception: " << e.what());
            }
        }

        LOG4CXX_DEBUG(logger, "Message to be sent: " << message);

        proxy.sendMessage(message);
    }
}

inline void debugArguments(const po::variables_map& vm) {
    LOG4CXX_DEBUG(logger, "Arguments:"
            << "\n\tclient: " << vm["client"].as<string>()
            << "\n\thost: " << vm["host"].as<string>()
            << "\n\tport: " << vm["port"].as<size_t>()
            << "\n\ttopic: " << (vm.count("topic") ? vm["topic"].as<string>() : "")
            << "\n\tschema: " << (vm.count("schema") ? vm["schema"].as<string>() : "")
            << "\n\tschema-path: " << vm["schema-path"].as<string>()
            << "\n\tlog: " << (vm.count("log") ? vm["log"].as<string>() : ""));
}
