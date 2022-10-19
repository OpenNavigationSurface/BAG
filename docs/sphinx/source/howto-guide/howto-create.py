import sys
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
            surf[column] = ((column * row) % kGridSize) + (column / float(kGridSize))
            # columnStart and columnEnd can be moved outside of the loop, but are here for simplicity
            columnStart: int = 0
            columnEnd: int = kGridSize - 1
            buffer: BAG.FloatLayerItems = BAG.FloatLayerItems(surf)
            elevationLayer.write(row, columnStart, row, columnEnd, buffer)
except Exception as e:
    sys.exit(str(e))
finally:
    dataset.close()

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
    sys.exit(str(e))
finally:
    dataset.close()

dataset.close()
