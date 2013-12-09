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

#ifndef _LOG2KAFKA_SERIALIZER_HH_
#define _LOG2KAFKA_SERIALIZER_HH_

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <sstream>

#include <avro/AvroSerialize.hh>
#include <avro/Compiler.hh>
#include <avro/DataFile.hh>
#include <avro/Decoder.hh>
#include <avro/Encoder.hh>
#include <avro/Generic.hh>
#include <avro/Node.hh>
#include <avro/Specific.hh>
#include <avro/Types.hh>
#include <avro/Writer.hh>

#include <boost/array.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/xpressive/xpressive.hpp>

#include <log4cxx/logger.h>

#include "Constants.hh"
#include "Mapper.hh"

typedef boost::array<uint8_t, 4> Magic;
typedef boost::array<uint8_t, 16> DataBlockSync;
typedef std::map<std::string, std::vector<uint8_t>> Metadata;

/**
 * Class responsible for serializing text entries.
 */
class Serializer {
public:

	Serializer();
	virtual ~Serializer();

	/**
	 * Class constructor with direct schema definition initialization.
	 *
	 * After setting the schema configuration file path, the constructor
	 * invoke the #configure() method.
	 *
	 * @param configFilePath the file path to the schema configuration
	 *                       and mapping
	 */
	explicit Serializer(std::string configFilePath);

	/**
	 * Set the configuration file path.
	 */
	void configFilePath(std::string configFilePath);

	/**
	 * Return the configuration file path.
	 */
	const std::string& configFilePath() const;

	/**
	 * Read de configuration file and load the schema mapper to use for
	 * serialization.
	 */
	void configure();

	/**
	 * Serialize a input text using the schema and mapper defined for the
	 * instance.
	 *
	 * @param entry the input text to serialize
	 * @param data the output data buffer
	 */
	void serialize(const std::string& entry, std::auto_ptr<avro::OutputStream>& data);

private:
	/**
	 * Class logger.
	 */
	static log4cxx::LoggerPtr logger;

	/**
	 * Text that mark the beginning of the AVRO schema definition in the
	 * configuration file.
	 */
	static const std::string schemaMarker;

	/**
	 * Schema and pattern mapper configuration file.
	 */
	std::string _configFilePath;

	/**
	 * AVRO Schema mapper.
	 */
	Mapper _mapper;

	DataBlockSync _sync;

	Metadata _metadata;

	/* methods */

	/**
	 * Load a schema mapper according to the definition read from the input
	 * stream.
	 *
	 * @param is the input stream to read.
	 */
	void loadMapper(std::istream &is);

	void writeHeader(avro::EncoderPtr& e);

	void writeDataBlock(avro::EncoderPtr& e, const avro::GenericDatum& datum, int64_t byteCount);

	void setMetadata(const std::string& key, const std::string& value);

	DataBlockSync makeSync();

	/**
	 * Display schema instance debug information.
	 *
	 * @param schema the AVRO schema instance to display.
	 */
	void debugSchemaNode(const avro::ValidSchema &schema) const;
};

#endif /* _LOG2KAFKA_SERIALIZER_HH_ */
