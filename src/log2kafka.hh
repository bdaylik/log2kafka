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

#ifndef _LOG2KAFKA_HH_
#define _LOG2KAFKA_HH_

#define BUILD_NAME "log2kafka"
#define log2kafka_VERSION_MAJOR 1
#define log2kafka_VERSION_MINOR 0
#define log2kafka_VERSION "1.0"

#include <iostream>
#include <memory>
#include <string>

#include <avro/Encoder.hh>
#include <avro/Decoder.hh>

#include <boost/program_options.hpp>

#include <libkafka/Client.h>

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/helpers/exception.h>

#include "ClientProxy.hh"
#include "Constants.hh"
#include "Serializer.hh"

#endif /* _LOG2KAFKA_HH_ */
