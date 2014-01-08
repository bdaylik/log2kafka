/**
 * @file ClientFacade.cc
 * @brief Client connection facade class.
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

namespace po = boost::program_options;
using namespace std;

#ifdef _LOG2KAFKA_USE_LOG4CXX_
using namespace log4cxx;

LoggerPtr ClientFacade::logger(Logger::getLogger("ClientFacade"));
#endif

/**
 * Default constructor.
 */
ClientFacade::ClientFacade() {
    initDefaults();
}

ClientFacade::~ClientFacade() {
    flush();
    rd_kafka_topic_destroy(kafkaTopic_);
    rd_kafka_destroy(kafkaClient_);
}

void ClientFacade::messageKey(std::string messageKey) {
    this->messageKey_ = messageKey;
}

void ClientFacade::topic(std::string topic) {
    vector<string> fields;

    boost::split(fields, topic, boost::is_any_of(":"));

    if (fields.size() >= 2) {
        try {
            partition_ = boost::lexical_cast<int>(fields[1]);
        }
        catch (exception& e) {
            LOG_WARN("Invalid partition value: " << fields[1] << ". Defaulting to: " << partition_);
        }
    }

    topic_ = fields[0];
}

void ClientFacade::serializer(const string& configFile) {
    unique_ptr<Serializer> serializer(new Serializer(configFile));
    this->serializer_ = move(serializer);
}

void ClientFacade::initDefaults() {
    partition_ = RD_KAFKA_PARTITION_UA;
}

void ClientFacade::configure(const boost::program_options::variables_map& vm) {

    topic(vm["kafka.topic"].as<string>());

    if (vm.count("schema")) {
        LOG_DEBUG("Schema defined. Using AVRO serialization mode");
        serializer(vm["schema"].as<string>());
    }

    /* Kafka configuration */

    kafkaConfig_ = rd_kafka_conf_new();
    kafkaTopicConfig_ = rd_kafka_topic_conf_new();
    rd_kafka_conf_res_t kafkaConfResult;
    char errstr[512];

    LOG_DEBUG("Setting kafka configuration");

    string key;
    ostringstream value;

    for (po::variables_map::const_iterator it = vm.begin(); it != vm.end(); ++it) {

        bool isKafkaClientOption = (it->first.find(Constants::KAFKA_CLIENT_OPTION_PREFIX) == 0);
        bool isKafkaTopicOption = (it->first.find(Constants::KAFKA_TOPIC_OPTION_PREFIX) == 0);

        if (isKafkaClientOption || isKafkaTopicOption) {

            if (isKafkaClientOption) {
                key = it->first.substr(Constants::KAFKA_CLIENT_OPTION_PREFIX.length());

                // 'topic' and 'key' are not valid librdkafka options
                if (key == "topic" || key == "key") continue;

            }
            else {
                key = it->first.substr(Constants::KAFKA_TOPIC_OPTION_PREFIX.length());
            }

            value.str("");

            if (typeid(int) == it->second.value().type()) {
                value << it->second.as<int>();
            }
            else {
                value << it->second.as<string>();
            }

            LOG_DEBUG("\t" << key << " = " << value.str());

            if (isKafkaClientOption) {
                kafkaConfResult = rd_kafka_conf_set(kafkaConfig_, key.data(), value.str().data(),
                    errstr, sizeof(errstr));
            }
            else {
                kafkaConfResult = rd_kafka_topic_conf_set(kafkaTopicConfig_, key.data(),
                    value.str().data(), errstr, sizeof(errstr));
            }

            if (kafkaConfResult != RD_KAFKA_CONF_OK) {
                throw ProducerCreationException(errstr);
            }
        }
    }

    /* Set up a message delivery report callback.
     * It will be called once for each message, either on successful
     * delivery to broker, or upon failure to deliver to broker.
     */
    rd_kafka_conf_set_dr_cb(kafkaConfig_, ClientFacade::deliverCallback);

    /* Create Kafka handle */

    if (!(kafkaClient_ = rd_kafka_new(RD_KAFKA_PRODUCER, kafkaConfig_, errstr, sizeof(errstr)))) {
        throw ProducerCreationException(errstr);
    }

    /* Prepare Kafka Topic */

    kafkaTopic_ = rd_kafka_topic_new(kafkaClient_, topic_.data(), kafkaTopicConfig_);

    /* Add brokers */

//    if (rd_kafka_brokers_add(kafkaClient_, broker.str().data()) == 0) {
//        throw InvalidBrokerException();
//    }
}

void ClientFacade::flush() {
    rd_kafka_poll(kafkaClient_, Constants::DEFAULT_CALLBACK_WAITING_TIMEOUT);
}

void ClientFacade::sendMessage(const string& message) {

    bool sendRawMessage = false;

    /* Prepare message */

    auto_ptr<avro::OutputStream> dataOutput = avro::memoryOutputStream();

    if (message.length() == 0) {
        LOG_WARN("Empty message entry discarded");
        return;
    }

    if (serializer_) { // Use serialization mode
        LOG_DEBUG("Schema defined. Using serialization mode");

        try {
            serializer_->serialize(message, dataOutput);
        }
        catch (exception& e) {
            sendRawMessage = true;
            LOG_ERROR("Using raw mode due to unexpected exception: " << e.what());
        }
    }
    else { // Use raw mode
        sendRawMessage = true;
        LOG_DEBUG("No schema defined. Using raw mode");
    }

    /* Send request */

    // Copy message key
    size_t keyLength = messageKey_.length();
    char* key = NULL;

    if (keyLength > 0) {
        key = new char[keyLength];
        memcpy(key, messageKey_.data(), keyLength);
    }

    // Copy message value
    size_t valueLength = 0;
    uint8_t* value = NULL;

    if (sendRawMessage) {
        valueLength = message.length();
        value = new uint8_t[valueLength];

        memcpy(value, message.data(), valueLength);
    }
    else {
        valueLength = dataOutput->byteCount();
        value = new uint8_t[valueLength];

        auto_ptr<avro::InputStream> dataInput = avro::memoryInputStream(*dataOutput);
        avro::StreamReader reader(*dataInput);
        reader.readBytes(value, valueLength);
    }

    if (Constants::IS_DEBUG_ENABLED) {
        LOG_DEBUG("MESSAGE");

        // The entire message is not printed with standard cout mechanism
        // due to NULL character interpretation
        cout.write(reinterpret_cast<const char*>(value), valueLength);
        cout << endl;

        LOG_DEBUG("MESSAGE END");
    }

    /* Send/Produce message. */

    rd_kafka_produce(kafkaTopic_, partition_, RD_KAFKA_MSG_F_FREE, reinterpret_cast<char *>(value),
        valueLength, key, keyLength, NULL);

    LOG_DEBUG("Sent " << valueLength
        << " bytes to topic " << rd_kafka_topic_name(kafkaTopic_)
        << ":" << partition_);

    /* Poll to handle delivery reports */

    rd_kafka_poll(kafkaClient_, 0);

    if (key != NULL) delete key;

    // Clean forced above by RD_KAFKA_MSG_F_FREE option
    //delete value;
}

void ClientFacade::deliverCallback(rd_kafka_t *rk, void *payload, size_t len,
    rd_kafka_resp_err_t error_code, void *opaque, void *msg_opaque) {

    if (error_code) {
        // TODO: show message description instead of code number
        LOG_WARN("Message delivery failed with error code: " << error_code);
    }
    else {
        LOG_INFO("Message delivered (" << len << " bytes)");
    }
}

/**
 * Generate a unique number to be used as request correlation identification.
 */
int ClientFacade::generateCorrelationId() {
    hash<string> hash_fn;
    time_t now = time(NULL);
    return hash_fn(ctime(&now));
}
