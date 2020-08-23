#!/usr/local/bin/python3
'''
This is a slightly modified version of https://github.com/lorenmh/mnist_handwritten_json
The modifications include:
 - Adjustment of the LIMIT to only use the first 100 images in the test set
 - Centering the output images of the test set to 24x24 instead of 28x28 pixels.
    This is required as Convnet uses 24x24 images and this was the fastest way to achieve that result.
    We simply limit the output with this line below: if row not in [0,1,27,28]

Loren Howard - 1/13/2018

The file format is documented here:
    http://yann.lecun.com/exdb/mnist/

This script simply converts the files from http://yann.lecun.com/exdb/mnist/
into a simple JSON format;

The outputted JSON file is an array of objects with two fields:
    image: an array with 784 0-255 pixel values (28*28*1byte image)
    label: the corresponding label for this image
'''


import json
import struct


IMG_HEADER_FMT = '>IIII'
IMG_HEADER_SZ = 16

LBL_HEADER_FMT = '>II'
LBL_HEADER_SZ = 8

LBL_FMT = 'B'
LBL_SZ = 1

LIMIT = 100

JSON_INDENT = 0

def struct_unpack_file(struct_fmt, struct_sz, f):
    while True:
        bytes = f.read(struct_sz)
        if not bytes:
            break
        yield struct.unpack(struct_fmt, bytes)
        
def unpack(img_fname, lbl_fname, o_fname):
    print('Unpacking %s and %s and outputting as %s' % (img_fname, lbl_fname,
                                                        o_fname))
    img_file = open(img_fname, 'rb')
    lbl_file = open(lbl_fname, 'rb')

    img_header = img_file.read(IMG_HEADER_SZ)
    lbl_header = lbl_file.read(LBL_HEADER_SZ)

    _, num_img, num_row, num_col = struct.unpack(IMG_HEADER_FMT, img_header)
    _, num_lbl =                   struct.unpack(LBL_HEADER_FMT, lbl_header)

    if num_img != num_lbl:
        raise ValueError('number of labels != number of images')

    img_sz = num_row * num_col
    img_fmt = 'B' * img_sz

    img_gen = struct_unpack_file(img_fmt, img_sz, img_file)
    lbl_gen = struct_unpack_file(LBL_FMT, LBL_SZ, lbl_file)

    # Crop images
    # img_gen is a tuple of img_sz pixels
    # Crop image down from 28x28 to 24x24 by only keeping the middle of every row
    out_img = []
    if num_col == num_row and num_col == 28:
        print("Cropping images...")
        # print("%d rows with %d cols" % (num_row, num_col))
        for img in img_gen:
            curr_img = []
            for row in range(num_row):
                # print("old row:%s" % (str(img[(row*num_col):(row*num_col)+28])))
                # print("new row:%s" %(str(img[(row*num_col)+2:(row*num_col)+26])))
                if row not in [0,1,27,28]: # hacky way to exclude first and last two rows
                    curr_img.extend(img[(row*num_col)+2:(row*num_col)+26])

            out_img.append(curr_img)
    else:
        out_img = img_gen

    lst = [{'image': img, 'label': lbl} for img,[lbl] in zip(out_img, lbl_gen)]

    o_file = open(o_fname, 'w')
    json.dump(lst[:LIMIT], o_file, separators=(',', ':')) # indent=JSON_INDENT, for pretty print

    img_file.close()
    lbl_file.close()
    o_file.close()

unpack('test_img.ubyte', 'test_lbl.ubyte', 'mnist_handwritten_test.json')
