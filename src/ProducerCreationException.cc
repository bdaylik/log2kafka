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

#include "Exceptions.hh"

ProducerCreationException::ProducerCreationException(const char* message) :
        message_(message) {
}

ProducerCreationException::ProducerCreationException() :
        message_("Failed to create new kafka producer") {
}

const char* ProducerCreationException::what() const throw () {
    return message_;
}
