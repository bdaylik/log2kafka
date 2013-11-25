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

#include "ClientProxy.hh"

using namespace std;
using namespace log4cxx;

namespace kafka = LibKafka;

LoggerPtr ClientProxy::logger(Logger::getLogger("ClientProxy"));

ClientProxy::ClientProxy() {
    initDefaults();
}

ClientProxy::ClientProxy(const string& host, size_t port, const string& topic) {
    initDefaults();

    this->host = host;
    this->port = port;
    this->topics.push_back(topic);
}

ClientProxy::~ClientProxy() {
}

void ClientProxy::initDefaults() {
    this->host = Constants::DEFAULT_HOST;
    this->port = Constants::DEFAULT_PORT;
    this->clientId = Constants::DEFAULT_CLIENT_ID;
    this->requiredAcks = 1;
    this->timeout = 20;
}

kafka::Message* ClientProxy::createMessage(const string& message) {

    int crc = 0;
    int offset = -1;
    signed char magicByte = 1;
    signed char attributes = 0; // last three bits must be zero to disable gzip compression

    const char* defaultKey = "no_key";
    unsigned char* key;
    unsigned char* value;

    int keyLength = 6;
    int valueLength = message.length();

    key = new unsigned char[keyLength];
    memcpy(key, defaultKey, keyLength);

    value = new unsigned char[valueLength];
    memcpy(value, message.data(), valueLength);

    return new kafka::Message(crc, magicByte, attributes, keyLength, key, valueLength, value, offset, true);
}

kafka::ProduceMessageSet* ClientProxy::createMessageSet(const string& message) {

    vector<kafka::Message*> messages;

    kafka::Message *kafkaMessage = createMessage(message);
    messages.push_back(kafkaMessage);

    // sizeof(offset) + sizeof(messageSize) + messageSize
    int messageSetSize = sizeof(long int) + sizeof(int) + kafkaMessage->getWireFormatSize(false);

    kafka::MessageSet* messageSet = new kafka::MessageSet(messageSetSize, messages, true);

    messageSetSize = messageSet->getWireFormatSize(false);
    int partition = 0;

    return new kafka::ProduceMessageSet(partition, messageSetSize, messageSet, true);
}

kafka::TopicNameBlock<kafka::ProduceMessageSet>* ClientProxy::createRequestTopicNameBlock(const string& topic,
        const string& message) {

    int numMessages = 1;

    kafka::ProduceMessageSet** messageSetArray = new kafka::ProduceMessageSet*[numMessages];
    messageSetArray[0] = createMessageSet(message);

    return new kafka::TopicNameBlock<kafka::ProduceMessageSet>(topic, numMessages, messageSetArray, true);
}

kafka::ProduceRequest* ClientProxy::createProduceRequest(const string& message) {

    int correlationId = getCorrelationId();
    vector<std::string>::size_type numTopics = this->topics.size();

    kafka::TopicNameBlock<kafka::ProduceMessageSet>** topicArray =
            new kafka::TopicNameBlock<kafka::ProduceMessageSet>*[numTopics];

    for (vector<std::string>::size_type i = 0; i < numTopics; i++) {
        topicArray[i] = createRequestTopicNameBlock(this->topics[i], message);
    }

    return new kafka::ProduceRequest(correlationId, clientId, requiredAcks, timeout, numTopics, topicArray,
            true);
}

void ClientProxy::sendMessage(const string& message) {

    kafka::ProduceRequest *pr1 = createProduceRequest(message);

    if (pr1 != NULL) {
        LOG4CXX_DEBUG(logger, "ProduceRequest:\n" << *pr1);

        kafka::Client *c = new kafka::Client(host, port);
        kafka::ProduceResponse *pr2 = c->sendProduceRequest(pr1);

        if (pr2 != NULL) {
            LOG4CXX_DEBUG(logger, "ProduceResponse:\n" << *pr2);

            if (pr2->hasErrorCode()) {
                LOG4CXX_DEBUG(logger, "Unexpected sendProduceRequest error");
            }

            delete pr2;
        }

        delete pr1;
    }
}

int ClientProxy::getCorrelationId() {
    hash<std::string> hash_fn;
    time_t now = time(NULL);
    return hash_fn(ctime(&now));
}
