import sys
import time
import numpy as np

import argparse

import h5py
import tomop as tp


class Hdf5ReaderClass():
    '''
    classdocs
    This class is responsible for reading the raw data from hdf5. Index always start at zero.
    Similar but independent from pipeline (we want to be independent from global data
    which is not properly initialized for the publisher.)
    '''

    def __init__(self, hdf5FileName):
        self.hdf5FileName = hdf5FileName
        self.rawDataPathInHDF5 = '/exchange/data'
        self.flatDataPathInHDF5 = '/exchange/data_white'
        self.darkDataPathInHDF5 = '/exchange/data_dark'
        #Open hdf5, Prepare datasets
        self.hdf5file = h5py.File(self.hdf5FileName, 'r')
        #Open dark
        self.darkImagesDataset = self.hdf5file[self.darkDataPathInHDF5]
        self.numberOfDarks = self.darkImagesDataset.shape[0]
        #Open flat (white)
        self.flatImageDataset = self.hdf5file[self.flatDataPathInHDF5]
        self.totalNumberOfFlats = self.flatImageDataset.shape[0]
        #raw
        self.rawImagesDataset = self.hdf5file[self.rawDataPathInHDF5]
        self.numberOfProjections = self.rawImagesDataset.shape[0]
        #maybe zero degrees do not match zeroth index
        self.projectionsZeroDegreeIndex = 0

    def __del__(self):
        '''
        Deconstructor, close file
        '''
        self.hdf5file.close()
        #logger.info('hdf5 file closed')

    #Also the implementation shows the superiority of hdf5 compared to Tiff/DMP..
    def getDarkImage(self, index):
        if index >= self.numberOfDarks:
            print("dark image index: " + str(index) +
                  " not available, exiting..")
            return None
        return self.darkImagesDataset[index, :, :]

    def getFlatImage(self, index):
        if index >= self.totalNumberOfFlats:
            print("flat image index: " + str(index) +
                  " not available, exiting..")
            return None
        return self.flatImageDataset[index, :, :]

    def getRawImage(self, angularIndex):
        angularIndex += self.projectionsZeroDegreeIndex
        if angularIndex >= self.numberOfProjections + self.projectionsZeroDegreeIndex:
            #should never be here
            print(
                "max number of raw images exceeded without stitching, starting from first image"
            )
            angularIndex %= self.numberOfProjections
        #index should always be valid due to modulo
        return self.rawImagesDataset[angularIndex, :, :]


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description='Push a hdf5 data set to SliceRecon.')
    parser.add_argument('path', metavar='path', help='path to the data')
    parser.add_argument(
        '--host', default="localhost", help='the projection server host')
    parser.add_argument(
        '--port', type=int, default=5558, help='the projection server port')
    parser.add_argument('--offset', type=int, default=0, help='offset')
    parser.add_argument('--count', type=int, default=0, help='count')
    parser.add_argument('--time', type=float, default=0.0, help='time interval')
    parser.add_argument('--projs', type=int, help='proj count')

    args = parser.parse_args()

    hdf5Reader = Hdf5ReaderClass(args.path)

    proj = hdf5Reader.getRawImage(0)
    rows, cols = proj.shape

    pub = tp.publisher(args.host, args.port)
    pub.send(
        tp.scan_settings_packet(0, hdf5Reader.numberOfDarks,
                                hdf5Reader.totalNumberOfFlats // 2, False))

    r = max(rows, cols)
    window_min_point = [-cols // 2, -cols // 2, -rows // 2]
    window_max_point = [cols // 2, cols // 2, rows // 2]

    pub.send(
        tp.geometry_specification_packet(0, window_min_point,
                                         window_max_point))
    angles = np.linspace(0, np.pi, args.projs, endpoint=False)
    pub.send(
        tp.parallel_beam_geometry_packet(0, rows, cols, args.projs, angles))

    for i in range(hdf5Reader.numberOfDarks):
        packet_dark = tp.projection_packet(
            0, i, [rows, cols],
            np.ascontiguousarray(hdf5Reader.getDarkImage(i).flatten()))
        pub.send(packet_dark)
        print("sent dark", i)
        time.sleep(args.time)

    for i in range(hdf5Reader.totalNumberOfFlats // 2):
        packet_flat = tp.projection_packet(
            1, i, [rows, cols],
            np.ascontiguousarray(hdf5Reader.getFlatImage(i).flatten()))
        pub.send(packet_flat)
        print("sent flat", i)
        time.sleep(args.time)

    for i in range(
            args.offset, args.offset + args.count
            if args.count > 0 else hdf5Reader.numberOfProjections):
        packet_raw = tp.projection_packet(
            2, i, [rows, cols],
            np.ascontiguousarray(hdf5Reader.getRawImage(i).flatten()))
        pub.send(packet_raw)
        print("sent proj", i)
        time.sleep(args.time)
