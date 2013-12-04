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

#include "ClientProxy.hh"

using namespace std;
using namespace log4cxx;

namespace kafka = LibKafka;

LoggerPtr ClientProxy::logger(Logger::getLogger("ClientProxy"));

ClientProxy::ClientProxy() {
    initDefaults();
}

ClientProxy::~ClientProxy() {
}

void ClientProxy::clientId(std::string clientId) {
    this->_clientId = clientId;
}

void ClientProxy::messageKey(std::string messageKey) {
    this->_messageKey = messageKey;
}

void ClientProxy::host(std::string host) {
    this->_host = host;
}

void ClientProxy::port(int port) {
    this->_port = port;
}

void ClientProxy::topic(std::string topic) {
    vector<string> fields;
    int partition = 0;

    // TODO: document pre condition: topic name present

    boost::split(fields, topic, boost::is_any_of(":"));

    if (fields.size() >= 2) {
        try {
            partition = boost::lexical_cast<int>(fields[1]);
        }
        catch (exception& e) {
            LOG4CXX_WARN(logger, "Invalid partition value: " << fields[1] << ". Defaulting to: " << partition);
        }
    }

    this->_topics.insert(make_pair(fields[0], partition));
}

void ClientProxy::requiredAcks(int requiredAcks) {
    this->_requiredAcks = requiredAcks;
}

void ClientProxy::timeoutAcks(int timeoutAcks) {
    this->_timeoutAcks = timeoutAcks;
}

void ClientProxy::serializer(const string& configFile) {
    unique_ptr<Serializer> serializer(new Serializer(configFile));
    this->_serializer = move(serializer);
}

void ClientProxy::initDefaults() {
    this->_clientId = Constants::DEFAULT_CLIENT_ID;
    this->_requiredAcks = Constants::DEFAULT_REQUIRED_ACKS;
    this->_timeoutAcks = Constants::DEFAULT_TIMEOUT_ACKS;
}

kafka::Message* ClientProxy::createMessage(const std::vector<uint8_t>& message) {

    int crc = 0;
    int offset = -1;
    signed char magicByte = 1;
    signed char attributes = 0; // last three bits must be zero to disable gzip compression

    int keyLength = this->_messageKey.length();
    int valueLength = message.size();

    uint8_t* key = new uint8_t[keyLength];
    memcpy(key, this->_messageKey.data(), keyLength);

    uint8_t* value = new uint8_t[valueLength + 1];
    copy(message.begin(), message.end(), value);
    value[valueLength] = '\0';

    //memcpy(value, message.data(), valueLength);

    return new kafka::Message(crc, magicByte, attributes, keyLength, key, valueLength, value, offset, true);
}

kafka::ProduceMessageSet* ClientProxy::createMessageSet(int partition, const std::vector<uint8_t>& message) {

    vector<kafka::Message*> messages;

    kafka::Message *kafkaMessage = createMessage(message);
    messages.push_back(kafkaMessage);

    // sizeof(offset) + sizeof(messageSize) + messageSize
    int messageSetSize = sizeof(long int) + sizeof(int) + kafkaMessage->getWireFormatSize(false);
    kafka::MessageSet* messageSet = new kafka::MessageSet(messageSetSize, messages, true);
    messageSetSize = messageSet->getWireFormatSize(false);

    return new kafka::ProduceMessageSet(partition, messageSetSize, messageSet, true);
}

kafka::TopicNameBlock<kafka::ProduceMessageSet>* ClientProxy::createRequestTopicNameBlock(const string& topic,
        int partition, const std::vector<uint8_t>& message) {

    int numMessages = 1;

    kafka::ProduceMessageSet** messageSetArray = new kafka::ProduceMessageSet*[numMessages];
    messageSetArray[0] = createMessageSet(partition, message);

    return new kafka::TopicNameBlock<kafka::ProduceMessageSet>(topic, numMessages, messageSetArray, true);
}

kafka::ProduceRequest* ClientProxy::createProduceRequest(const std::vector<uint8_t>& message) {

    int correlationId = generateCorrelationId();
    vector<std::string>::size_type numTopics = this->_topics.size();

    kafka::TopicNameBlock<kafka::ProduceMessageSet>** topicArray =
            new kafka::TopicNameBlock<kafka::ProduceMessageSet>*[numTopics];

    int i = 0;

    for (topicmap::iterator it = this->_topics.begin(); it != this->_topics.end(); ++it) {
        topicArray[i] = createRequestTopicNameBlock(it->first, it->second, message);
        i++;
    }

    return new kafka::ProduceRequest(correlationId, _clientId, _requiredAcks, _timeoutAcks, numTopics,
            topicArray, true);
}

void ClientProxy::sendMessage(const string& message) {

    LOG4CXX_DEBUG(logger, "Message: " << message);

    std::vector<uint8_t> preparedMessage;

    /* Prepare message */

    if (message.length() == 0) {
        LOG4CXX_WARN(logger, "Empty message entry discarded");
        return;
    }

    if (_serializer) { // Use serialization mode
        LOG4CXX_DEBUG(logger, "Schema defined. Using serialization mode");

        try {
            _serializer->serialize(message, preparedMessage);
        }
        catch (exception& e) {
            copy(message.begin(), message.end(), back_inserter(preparedMessage));
            LOG4CXX_ERROR(logger, "Using raw mode due to unexpected exception: " << e.what());
        }
    }
    else { // Use raw mode
        copy(message.begin(), message.end(), back_inserter(preparedMessage));
        LOG4CXX_DEBUG(logger, "No schema defined. Using raw mode");
    }

    if (LOG4CXX_UNLIKELY(logger->isDebugEnabled())) {
        LOG4CXX_DEBUG(logger, "Message to be sent:");
        LOG4CXX_DEBUG(logger, "MESSAGE BEGIN");
        cout.write(reinterpret_cast<const char*>(preparedMessage.data()), preparedMessage.size());
        cout << endl;
        LOG4CXX_DEBUG(logger, "MESSAGE END");
    }

    /* Send request */

    kafka::ProduceRequest *pr1 = createProduceRequest(preparedMessage);

    if (pr1 != NULL) {
        LOG4CXX_DEBUG(logger, "ProduceRequest:\n" << *pr1);

        kafka::Client *c = new kafka::Client(_host, _port);
        kafka::ProduceResponse *pr2 = c->sendProduceRequest(pr1);

        if (pr2 != NULL) {
            LOG4CXX_DEBUG(logger, "ProduceResponse:\n" << *pr2);

            if (pr2->hasErrorCode()) {
                LOG4CXX_DEBUG(logger, "Unexpected sendProduceRequest error");
            }

            //delete pr2;
        }

        //delete pr1;
    }
}

int ClientProxy::generateCorrelationId() {
    hash<std::string> hash_fn;
    time_t now = time(NULL);
    return hash_fn(ctime(&now));
}
