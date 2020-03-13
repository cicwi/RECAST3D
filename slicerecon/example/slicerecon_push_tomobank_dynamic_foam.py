#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
import tomop as tp
import numpy as np
import argparse
import h5py
import dxchange
import dxchange.reader as dxreader
import matplotlib.pyplot as plt
import tomopy

'''
    Example file for pushing the TomoBank dynamic foam dataset.
    see: https://tomobank.readthedocs.io/en/latest/source/data/docs.data.dynamic.html
    
    This file reads a HDF5 file, parses projections with Tomopy and sends them to SliceRecon server. The file has been
    written to send the specific "dynamic foam" dataset, but could well serve as inspiration to send other HDF5 types.

    Some of the code in this file is taken from tomopy_rectv.py, read_continuous.py which is found
    on https://github.com/math-vrn/rectv_gpu by math-vrn, licensed with BSD 2-Simplified.
    
    Other code is taken from slicerecon_push_flexdata.py in this repo, written by
    J. Buurlage, licensed with GPL 3.
    
    Call this file with,
      slicerecon_push_aps32id.py /path/to/dk_MCFG_1_p_s1_.h5
    
    Adriaan
'''


def get_dx_dims(fname, dataset):
    """
    Read array size of a specific group of Data Exchange file.ls 

    Parameters
    ----------
    fname : str
        String defining the path of file or file name.
    dataset : str
        Path to the dataset inside hdf5 file where data is located.

    Returns
    -------
    ndarray
        Data set size.
    """

    grp = '/'.join(['exchange', dataset])

    with h5py.File(fname, "r") as f:
        try:
            data = f[grp]
        except KeyError:
            return None

        shape = data.shape

    return shape


def main(arg):
    parser = argparse.ArgumentParser(description='Push a dxchange/HDF5 data set to Slicerecon.')
    parser.add_argument("fname", help="file name of a tomographic dataset")
    parser.add_argument("--type", nargs='?', type=str, default="slice",
                        help="reconstruction type: full (default slice)")
    parser.add_argument("--nsino", nargs='?', type=float, default=0.5,
                        help="location of the sinogram used by slice reconstruction (0 top, 1 bottom): 0.5 (default 0.5)")
    parser.add_argument("--nsinoheight", nargs='?', type=int, default=16,
                        help="amount of vertical detector pixels to use, has to be a multiple of 2")
    parser.add_argument("--tv", nargs='?', type=bool, default=False,
                        help="Use Total variation reconstruction method (Gridrec otherwise): False (default False)")
    parser.add_argument("--frame", nargs='?', type=str, default=92, help="time frame with motion: 92 (default 92)")
    parser.add_argument('--dataset', default='data',
                        help='which dataset to use from the HDF5 file, {data,data_dark,data_white}')
    parser.add_argument('--binning', nargs='?', type=int, default=1, help="binning factor")
    parser.add_argument('--sample', type=int, default=1,
                        help='the binning to use on the detector, and how many projections to skip')
    parser.add_argument('--host', default="localhost", help='the projection server host')
    parser.add_argument('--port', type=int, default=5558, help='the projection server port')
    parser.add_argument('--skipgeometry', action='store_true', help='assume the geometry packet is already sent')
    args = parser.parse_args()

    fname = args.fname

    if not os.path.isfile(fname):
        print("HDF5/dx file not found.")
        return

    nsino = float(args.nsino)
    nsinoheight = int(args.nsinoheight)
    binning = int(args.binning)
    subsampling = int(args.sample)  # taking 1 per (subsampling) instead of all the frames

    nproj = 600  # number of projections per 180 degrees interval, this is coded
    scene_id = 0

    assert((nproj / subsampling).is_integer())
    assert((nsinoheight/2).is_integer())
    proj_count = int(nproj / subsampling)  # this should be the --group-size on the server, because we want
                                           # reconstructions from a sampled 180 shot
    print("Note: call the SliceRecon server with --group-size", proj_count, "to get reconstruction from 180 degrees angles")

    data_size = get_dx_dims(fname, 'data')

    # Select sinogram range to reconstruct.
    ssino = int(data_size[1] * nsino)
    sino_start = ssino-nsinoheight/2*pow(2, binning)
    sino_end = ssino+nsinoheight/2*pow(2, binning)
    sino = (int(sino_start), int(sino_end))

    # Read APS 32-BM raw data, for the sake of darks and flats
    print("Reading flats, darks ...")
    proj, flat, dark, _ = dxchange.read_aps_32id(fname, proj=1, sino=sino) # angles give nonsense values

    # Phase retrieval for tomobank id 00080
    # sample_detector_distance = 25
    # detector_pixel_size_x = 3.0e-4
    # monochromator_energy = 16

    # Phase retrieval
    # data = tomopy.prep.phase.retrieve_phase(data,pixel_size=detector_pixel_size_x,dist=sample_detector_distance,energy=monochromator_energy,alpha=8e-03,pad=True)

    # @todo Fix center of rotation shift!
    # rot_center = data.shape[2]/2

    # Detector dimensions
    rows = proj.shape[1]
    cols = proj.shape[2]

    # parallel beam data is easy, the geometry volume is never bigger than the projection volume
    rx = np.ceil(data_size[2] / 2)  # max radius in the x,y plane
    rz = np.ceil(data_size[1] / 2)  # max radius in the z axis

    pub = tp.publisher(args.host, args.port)

    if not args.skipgeometry:
        window_min_point = [-rx, -rx, -rz]  # x,y,z
        window_max_point = [rx, rx, rz]  # x,y,z

        angles = np.linspace(0, 2*np.pi, proj_count, endpoint=False) # np.mod(theta[0:nproj], np.pi)

        pub.send(tp.geometry_specification_packet(scene_id, window_min_point, window_max_point))
        pub.send(tp.parallel_beam_geometry_packet(scene_id, rows, cols, proj_count, angles))

        # We're not sending flats and darks to the SliceRecon server (see below) because (i) buffer may not be large
        # enough and (ii) we will want to do preprocessing of the projection data here anyway
        
        # already_linear_flatdarks = False
        # pub.send(tp.scan_settings_packet(scene_id, dark.shape[0], flat.shape[0], already_linear_flatdarks))
        # for i in np.arange(0, 2):
        #     pub.send(tp.projection_packet(0, i, [rows, cols], np.ascontiguousarray(dark[i, :, :].flatten())))
        #
        # for i in np.arange(0, 2):
        #     pub.send(tp.projection_packet(1, i, [rows, cols], np.ascontiguousarray(flat[i, :, :].flatten())))

    # I'm circumventing the dxchange.read_aps_32id, as it cannot select specific projections (always loading the full
    # dataset)
    exchange_base = "exchange"
    tomo_grp = '/'.join([exchange_base, 'data'])

    j = 0
    for i in np.arange(1, data_size[0], subsampling):
        print("Pushing ", i, " of ", data_size[0])
        data = dxreader.read_hdf5(fname, tomo_grp, slc=((int(i),int(i)+1), sino))

        # Flat-field correction of raw data.
        data = tomopy.normalize(data, flat, dark)

        # Remove stripes (not so suitable for realtime really!)
        # data = tomopy.remove_stripe_fw(data, level=7, wname='sym16', sigma=1, pad=True)

        # Log filter
        # data = tomopy.minus_log(data)
        # data = tomopy.remove_nan(data, val=0.0)
        # data = tomopy.remove_neg(data, val=0.00)
        # data[np.where(data == np.inf)] = 0.00

        packet_type = 2 # projection packet
        pub.send(tp.projection_packet(packet_type, j, [rows, cols], np.ascontiguousarray(data[0].flatten())))
        j = j+1


        # Here is a validation FBP-gridrec reconstruction, locally with TomoPy. 
        # rec = tomopy.recon(
        #     data[time_frame*nproj:(time_frame+1)*nproj],
        #     theta[time_frame*nproj:(time_frame+1)*nproj],
        #     # center=rot_center-np.mod(time_frame, 2),
        #     center=rot_center,
        #     algorithm='gridrec')
        # rec = tomopy.circ_mask(rec0, axis=0, ratio=0.95)

        # Plot projection data
        # plt.figure(1)
        # plt.imshow(data[0])
        # plt.pause(0.001)


if __name__ == "__main__":
    main(sys.argv[1:])
