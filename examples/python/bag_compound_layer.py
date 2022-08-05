import os
import sys
import argparse
import logging

import numpy as np

import bagPy as BAG


logger = logging.getLogger(__name__)


kGridSize: int = 100
kSepSize: int = 3


def main():
    parser = argparse.ArgumentParser(description='Demonstrate creation of compound metadata layers using Python API.')
    parser.add_argument('xmlFileName', metavar='xmlFileName', help='File containing XML metadata as input')
    parser.add_argument('outFileName', metavar='outFileName', help='File to write BAG to')
    args = parser.parse_args()

    log_config = {'level': logging.INFO}
    logging.basicConfig(**log_config)

    # Initial construction from the XML metadata example file provided.
    logger.info("Creating the BAG from XML file metadata, ")

    metadata: BAG.Metadata = BAG.Metadata()

    try:
        metadata.loadFromFile(args.xmlFileName)
    except BAG.ErrorLoadingMetadata as e:
        return e.what()

    # Create the dataset.
    dataset: BAG.Dataset = None
    try:
        chunkSize: int = 100
        compressionLevel: int = 1

        dataset = BAG.Dataset.create(args.outFileName, metadata, chunkSize, compressionLevel)
    except FileExistsError as e:
        mesg = f"Unable to create BAG '{args.outFileName}'; file already exists."
        logger.error(mesg)
        return mesg

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
        return str(e)

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
            # can we just pass a numpy array to a LayerItem?!?!
            buffer: BAG.FloatLayerItems = BAG.FloatLayerItems(uncert)
            uncertaintyLayer.write(row, columnStart, row, columnEnd, buffer)
    except Exception as e:
        return str(e)

    # Add a compound layer (additional metadata) for elevation.
    # Using Figure 4 from the specification as an example
    # see https://github.com/OpenNavigationSurface/BAG/issues/2

    try:
        simpleLayerName: str = elevationLayer.getDescriptor().getName()
        chunkSize: int = 100
        compressionLevel: int = 1
        compoundLayer: BAG.CompoundLayer = dataset.createMetadataProfileCompoundLayer(
            BAG.NOAA_NBS_2022_06_METADATA_PROFILE,
            simpleLayerName, chunkSize, compressionLevel)
        # At this point, all entries in the compound layer point to index 0,
        # which is a no data value.

        # Write a couple records.
        record: BAG.Record = BAG.CreateRecord_NOAA_NBS_2022_06(
            3,          # data_assessment
            False,      # significant_features
            True,       # feature_least_depth
            1234.567,   # feature_size
            True,       # coverage
            False,      # bathy_coverage
            9.87,       # horizontal_uncert_fixed
            1.23,       # horizontal_uncert_var
            0.98,       # vertical_uncert_fixed
            0.12,       # vertical_uncert_var
            'Creative Commons Zero Public Domain Dedication (CC0)', # license_Name
            'https://creativecommons.org/publicdomain/zero/1.0/',   # license_URL
            'CD71EB77-5812-4735-B728-0DC1AE2A2F3B',                 # source_Survey_ID
            'NOAA',                                                 # source_Institution
            '2019-04-01 00:00:00.0Z',                               # survey_data_start
            '2019-04-01 12:00:00.0Z'                                # survey_date_end
        )

        valueTable: BAG.ValueTable = compoundLayer.getValueTable()

        # Store the new record in memory and in the BAG.
        firstRecordIndex: int = valueTable.addRecord(record)

        record: BAG.Record = BAG.CreateRecord_NOAA_NBS_2022_06(
            1,          # data_assessment
            True,       # significant_features
            False,      # feature_least_depth
            987.6,      # feature_size
            False,      # coverage
            True,       # bathy_coverage
            12345.67,   # horizontal_uncert_fixed
            89.0,       # horizontal_uncert_var
            0.12,       # vertical_uncert_fixed
            0.89,       # vertical_uncert_var
            'Open Data Commons Public Domain Dedication and Licence (PDDL)', # license_Name
            'http://opendatacommons.org/licenses/pddl/1.0/',                # license_URL
            '15B46F99-1D94-4669-92D8-AA86F533B097',                 # source_Survey_ID
            'NOAA',                                                 # source_Institution
            '2019-04-02 00:00:00.0Z',                               # survey_data_start
            '2019-04-02 12:00:00.0Z'                                # survey_date_end
        )

        # Store the new record in memory and in the BAG.
        secondRecordIndex: int = valueTable.addRecord(record)

        numRows, numColumns = dataset.getDescriptor().getDims()

        # Set up the compound layer to point to the new records.
        # Let's say the first 5 rows of elevation should use the first record
        # index, and the next 3 columns use the second record index.

        # Start at row 0, go to (including) row 4.
        # Use the entire column.
        rowStart: int = 0
        columnStart: int = 0
        rowEnd: int = 4
        columnEnd: int = numColumns - 1

        # Create the buffer.  The type depends on the indexType used when
        # creating the compound layer.
        # The buffer contains the first record's index covering the first four
        # rows (across all the columns).
        numElements: int = (rowEnd - rowStart + 1) * numColumns
        firstBuffer: np.ndarray = np.full(numElements, firstRecordIndex)

        # can we just pass a numpy array to a LayerItem?!?!
        buffer: BAG.UInt16LayerItems = BAG.UInt16LayerItems(firstBuffer)
        compoundLayer.write(rowStart, columnStart, rowEnd, columnEnd,
                            buffer)

        # Start at row 6, go to the last row.
        # Start at column 0, go to (including) column 2.
        rowStart = 5
        columnStart = 0
        rowEnd = numRows - 1
        columnEnd = 2

        # Create the buffer.  The type depends on the indexType used when
        # creating the compound layer.
        # The buffer contains the second record's index covering the first four
        # rows (across all the columns).
        numElements = (rowEnd - rowStart + 1) * (columnEnd - columnStart + 1)
        secondBuffer: np.ndarray = np.full(numElements, secondRecordIndex)

        # can we just pass a numpy array to a LayerItem?!?!
        buffer: BAG.UInt16LayerItems = BAG.UInt16LayerItems(secondBuffer)
        compoundLayer.write(rowStart, columnStart, rowEnd, columnEnd,
                            buffer)

        # Read the data back.
        # Get the compound layer records specified by the fifth and sixth rows,
        # second and third columns.

        rowStart = 4        # fifth row
        columnStart = 1     # second column
        rowEnd = 5          # sixth row
        columnEnd = 2       # third column

        buff = compoundLayer.read(rowStart, columnStart, rowEnd,
                                  columnEnd)
        print(buff)

    except Exception as e:
        return str(e)


    return os.EX_OK


if __name__ == '__main__':
    sys.exit(main())
