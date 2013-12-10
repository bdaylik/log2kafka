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

#ifndef _LOG2KAFKA_EXCEPTION_HH_
#define _LOG2KAFKA_EXCEPTION_HH_

#include <exception>

/**
 * Indicate an invalid broker definition.
 */
class InvalidBrokerException: public std::exception {

    /**
     * Return the exception description message.
     */
    virtual const char* what() const throw ();
};

/**
 * Indicate an invalid mapper, one without a regular expression pattern
 * defined.
 */
class InvalidMapperException: public std::exception {

    /**
     * Return the exception description message.
     */
    virtual const char* what() const throw ();
};

/**
 * Indicate a mismatch between a received entry and the schema mapper
 * selected to process it.
 */
class MapperMatchException: public std::exception {

    /**
     * Return the exception description message.
     */
    virtual const char* what() const throw ();
};

/**
 * Indicate a failure to create a new producer.
 */
class ProducerCreationException: public std::exception {
public:
    ProducerCreationException();

    /**
     * Class constructor.
     *
     * @param message the exception description message
     */
    explicit ProducerCreationException(const char* message);

    /**
     * Return the exception description message.
     */
    virtual const char* what() const throw ();

private:
    const char* message_;
};

#endif /* _LOG2KAFKA_EXCEPTION_HH_ */
