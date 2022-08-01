import os
import sys
import argparse
import logging

import bagPy as BAG


logger = logging.getLogger(__name__)


def main():
    parser = argparse.ArgumentParser(description='Demonstrate creation of compound metadata layers using Python API.')
    parser.add_argument('xml_file_name', metavar='xmlFileName', help='File containing XML metadata as input')
    parser.add_argument('out_file_name', metavar='outFileName', help='File to write BAG to')
    args = parser.parse_args()

    log_config = {'level': logging.INFO}
    logging.basicConfig(**log_config)

    # Initial construction from the XML metadata example file provided.
    logger.info("Creating the BAG from XML file metadata, ")

    metadata: BAG.Metadata = BAG.Metadata()

    try:
        metadata.loadFromFile(args.xml_file_name)
    except BAG.ErrorLoadingMetadata as e:
        logger.info(type(e))

    return os.EX_OK


if __name__ == '__main__':
    sys.exit(main())
