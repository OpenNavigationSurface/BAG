import os
import sys
import struct

import numpy as np

import bagPy as BAG


kGridSize: int = 100
kSepSize: int = 3

xmlFileName: str = sys.argv[1]
outFileName: str = sys.argv[2]
    
# Read metadata from XML file
metadata: BAG.Metadata = BAG.Metadata()

try:
		metadata.loadFromFile(xmlFileName)
except BAG.ErrorLoadingMetadata as e:
		sys.exit(e.what())

# Create the dataset.
dataset: BAG.Dataset = None
try:
		chunkSize: int = 100
		compressionLevel: int = 1

		dataset = BAG.Dataset.create(outFileName, metadata, chunkSize, compressionLevel)
except FileExistsError as e:
		mesg = f"Unable to create BAG '{outFileName}'; file already exists."
		sys.exit(mesg)

# Write the elevation layer, constructing bogus data as we do so.
surfRange = (-10.0, -10.0 - ((kGridSize - 1) * (kGridSize - 1) + kGridSize) / 10.0)
elevationLayer: BAG.SimpleLayer = dataset.getSimpleLayer(BAG.Elevation)
pDescriptor: BAG.LayerDescriptor = elevationLayer.getDescriptor()
pDescriptor.setMinMax(surfRange[0], surfRange[1])
elevationLayer.writeAttributes()
# Write the data.
surf: np.ndarray = np.zeros(kGridSize)
try:
		for row in range(kGridSize):
				# Generate a row's worth of data in surf and write to layer
				for column in range(kGridSize):
						surf[column] = ((column * row) % kGridSize) + \
								(column / float(kGridSize))
				# columnStart and columnEnd can be moved outside of the loop, but are here for simplicity
				columnStart: int = 0
				columnEnd: int = kGridSize - 1
				# can we just pass a numpy array to a LayerItem?!?!
				buffer: BAG.FloatLayerItems = BAG.FloatLayerItems(surf)
				elevationLayer.write(row, columnStart, row, columnEnd, buffer)
except Exception as e:
		dataset.close()
		sys.exit(str(e))

# Write the uncertainty layer, constructing bogus data as we do so.
uncertRange = (-1.0, -1.0 - ((kGridSize - 1) * (kGridSize - 1) + kGridSize) / 100.0)
uncertaintyLayer: BAG.SimpleLayer = dataset.getSimpleLayer(BAG.Uncertainty)
pDescriptor = uncertaintyLayer.getDescriptor()
pDescriptor.setMinMax(uncertRange[0], uncertRange[1])
uncertaintyLayer.writeAttributes()
# Write the data.
uncert: np.ndarray = np.zeros(kGridSize)
try:
		for row in range(kGridSize):
				# Generate a row's worth of data in uncert and write to layer
				for column in range(kGridSize):
						uncert[column] = ((column * row) % kGridSize) / 1000.0
				# columnStart and columnEnd can be moved outside of the loop, but are here for simplicity
				columnStart: int = 0
				columnEnd: int = kGridSize - 1
				buffer: BAG.FloatLayerItems = BAG.FloatLayerItems(uncert)
				uncertaintyLayer.write(row, columnStart, row, columnEnd, buffer)
except Exception as e:
		dataset.close()
		sys.exit(str(e))

# Add a georeferenced metadata layer (additional metadata) for elevation.
try:
		indexType = BAG.DT_UINT16;
		simpleLayerName: str = elevationLayer.getDescriptor().getName()
		chunkSize: int = 100
		compressionLevel: int = 1
		
		definition = BAG.RecordDefinition(5)
		definition[0].name = "temporal_variability"
		definition[0].type = BAG.DT_UINT32
		definition[1].name = "full_coverage"
		definition[1].type = BAG.DT_BOOLEAN
		definition[2].name = "feature_size"
		definition[2].type = BAG.DT_FLOAT32
		definition[3].name = "survey_date_start"
		definition[3].type = BAG.DT_STRING
		definition[4].name = "survey_date_end"
		definition[4].type = BAG.DT_STRING
		georefMetaLayer: BAG.GeorefMetadataLayer = dataset.createMetadataProfileGeorefMetadataLayer(indexType,
				BAG.UNKNOWN_METADATA_PROFILE, simpleLayerName, definition,
				chunkSize, compressionLevel)
				
		# At this point, all entries in the georeferenced metadata layer point to index 0,
		# which is a no data value.
		valueTable: BAG.ValueTable = georefMetaLayer.getValueTable()
		# Write a couple records.
		
		# First metadata record
		record: BAG.Record = BAG.Record(5)
		record[0].assignUInt32(1)
		record[1].assignBool(True)
		record[2].assignFloat(9.87)
		record[3].assignString("2019-04-01 00:00:00.0Z")
		record[4].assignString("2019-04-01 12:00:00.0Z")
		# Store the new record in memory and in the BAG.
		firstRecordIndex: int = valueTable.addRecord(record)

		record: BAG.Record = BAG.Record(5)
		record[0].assignUInt32(6)
		record[1].assignBool(False)
		record[2].assignFloat(12345.67)
		record[3].assignString("2019-04-02 00:00:00.0Z")
		record[4].assignString("2019-04-02 12:00:00.0Z")
		# Store the new record in memory and in the BAG.
		secondRecordIndex: int = valueTable.addRecord(record)

		# Set up the georeferenced metadata layer to point to the new records.
		# Let's say the first 5 rows of elevation should use the first record
		# index, and the next 3 columns use the second record index.
		numRows, numColumns = dataset.getDescriptor().getDims()
		# Start at row 0, go to (including) row 4.
		# Use the entire column.
		rowStart: int = 0
		columnStart: int = 0
		rowEnd: int = 4
		columnEnd: int = numColumns - 1

		# Create the buffer.  The type depends on the indexType used when
		# creating the georeferenced metadata layer.
		# The buffer contains the first record's index covering the first four
		# rows (across all the columns).
		numElements: int = (rowEnd - rowStart + 1) * numColumns
		firstBuffer: np.ndarray = np.full(numElements, firstRecordIndex, dtype=np.ushort)

		buffer: BAG.UInt16LayerItems = BAG.UInt16LayerItems(firstBuffer)
		georefMetaLayer.write(rowStart, columnStart, rowEnd, columnEnd,
												buffer)

		# Start at row 6, go to the last row.
		# Start at column 0, go to (including) column 2.
		rowStart = 5
		columnStart = 0
		rowEnd = numRows - 1
		columnEnd = 2

		# Create the buffer.  The type depends on the indexType used when
		# creating the georeferenced metadata layer.
		# The buffer contains the second record's index covering the first four
		# rows (across all the columns).
		numElements = (rowEnd - rowStart + 1) * (columnEnd - columnStart + 1)
		secondBuffer: np.ndarray = np.full(numElements, secondRecordIndex, dtype=np.ushort)

		buffer: BAG.UInt16LayerItems = BAG.UInt16LayerItems(secondBuffer)
		georefMetaLayer.write(rowStart, columnStart, rowEnd, columnEnd,
												buffer)

		# Read the data back.
		# Get the georeferenced metadata layer records specified by the fifth and sixth rows,
		# second and third columns.
		rowStart = 4        # fifth row
		columnStart = 1     # second column
		rowEnd = 5          # sixth row
		columnEnd = 2       # third column

		buff = georefMetaLayer.read(rowStart, columnStart, rowEnd,
															columnEnd)
		# Cast from uint8_t into unint16_t
		buffer_raw = bytes([b for b in buff])
		buffer = struct.unpack_from(f"{len(buffer_raw)//2}H", buffer_raw)

		numElements = (rowEnd - rowStart + 1) * (columnEnd - columnStart + 1)
		records: BAG.Records = valueTable.getRecords()

		for i in range(numElements):
				recordIndex: int = buffer[i]

				# Retrieve values via the ValueTable::getValue().
				# Get survey_date_start by field name
				surveyDateStart: BAG.CompoundDataType = valueTable.getValue(recordIndex,
																																		"survey_date_start")
				print(f"survey_date_start is {surveyDateStart.asString()} from record index: {recordIndex}")

				# Get feature_size by field index.
				fieldIndex: int = valueTable.getFieldIndex("feature_size")
				featureSize: BAG.CompoundDataType = valueTable.getValue(recordIndex,
																																fieldIndex)
				print(f"feature_size is {featureSize.asFloat()} from record index: {recordIndex}")

				# Another way to grab the values using the records directly.
				# This only supports numerical indices.

				# Get survey_date_start.
				fieldIndex: int = valueTable.getFieldIndex("survey_date_start")
				surveyDateStart: BAG.CompoundDataType = records[recordIndex][fieldIndex]
				print(f"survey_date_start is {surveyDateStart.asString()} from record index: {recordIndex}")

				# Get feature_size.
				fieldIndex: int = valueTable.getFieldIndex("feature_size")
				featureSize: BAG.CompoundDataType = records[recordIndex][fieldIndex]
				print(f"feature_size is {featureSize.asFloat()} from record index: {recordIndex}")

except TypeError as e:
		sys.exit(f"TypeError: {str(e)}")
except Exception as e:
		sys.exit(str(e))
finally:
	dataset.close()
