/**
 * @file ClientFacade.hh
 * @brief Client connection facade class header.
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

#pragma once

#ifndef _LOG2KAFKA_CLIENT_FACADE_HH_
#define _LOG2KAFKA_CLIENT_FACADE_HH_

#include <string>
#include <cstring>
#include <ctime>
#include <functional>
#include <iomanip>
#include <memory>
#include <vector>

#include <boost/lexical_cast.hpp>

extern "C" {
#include <librdkafka/rdkafka.h>
}

#include "Serializer.hh"

/**
 * Client connection facade class.
 */
class ClientFacade {
public:

    ClientFacade();
    virtual ~ClientFacade();

    /*-- getters/setters --*/

    /**
     * Set the message key.
     */
    void messageKey(std::string messageKey);

    /**
     * Set the topic and possibly the partition.
     *
     * The format expected of the string is
     * <b>&lt;topic_name&gt;[:&lt;partition&gt;]</b>.
     * If omitted, a random partition will be selected.
     *
     * @pre The given topic value is not blank (empty or all spaces).
     */
    void topic(std::string topic);

    /**
     * Configure an AVRO serializer instance according to the specified
     * configuration file.
     *
     * @param configFile the file path to the schema configuration and mapping
     */
    void serializer(const std::string& configFile);

    /*-- methods --*/

    /**
     * Prepare and establish the kafka client connection.
     */
    void configure(const boost::program_options::variables_map& vm);

    /**
     * Flush message queue.
     */
    void flush();

    /**
     * Send a message to kafka.
     *
     * @param message the message to be sent
     */
    void sendMessage(const std::string& message);

private:

    /*-- static fields --*/

#ifdef _LOG2KAFKA_USE_LOG4CXX_
    /**
     * Class logger.
     */
    static log4cxx::LoggerPtr logger;
#endif

    /*-- fields --*/

    /**
     * The kafka client handle.
     */
    rd_kafka_t* kafkaClient_;

    /**
     * The kafka configuration object;
     */
    rd_kafka_conf_t* kafkaConfig_;

    /**
     * The kafka topic handle.
     */
    rd_kafka_topic_t* kafkaTopic_;

    /**
     * The kafka topic configuration object;
     */
    rd_kafka_topic_conf_t* kafkaTopicConfig_;

    /**
     * Kafka messake key.
     */
    std::string messageKey_;

    /**
     * Topic name.
     */
    std::string topic_;

    /**
     * Topic partition.
     * (Default: -1, random selection)
     */
    int partition_;

    /**
     * Serializer object to use.
     */
    std::unique_ptr<Serializer> serializer_;

    /*-- static methods --*/

    /**
     * Message delivery report callback.
     * Called once for each message.
     *
     * @see rdkafka.h
     */
    static void deliverCallback(rd_kafka_t* rk, void* payload, size_t len,
        rd_kafka_resp_err_t error_code,
        void* opaque, void* msg_opaque);

    /*-- methods --*/

    /**
     * Initialize members with default values.
     *
     * @see Constants
     */
    void initDefaults();

    /**
     * Generate an unique correlation id for a request.
     */
    int generateCorrelationId();
};

#endif /* _LOG2KAFKA_CLIENT_FACADE_HH_ */
