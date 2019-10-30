# BoomerCV

<img src="https://i.kym-cdn.com/photos/images/original/001/395/571/104.jpg" width="100" height="100" />

BoomerCV is a minimalistic collection of computer vision algorithms, made with simplicity, speed, and boomers in mind.

Setup and usage guides are for zoomers, but just in case...

## Installation

```sh
  git clone git@github.com:marvrez/BoomerCV.git
  cd BoomerCV/
  make -j
```

If any errors show up, it really do be like that sometimes. Otherwise everything should be working, but you should, just in case, check the next section.

## Usage
BoomerCV comes with some cool examples.
You can run them by executing the following command
```sh
  ./boomercv <function>
```

Where `<function>` is currently one of

* `grayscale` - grayscale a given input image
* `resize` - resize a given input image
* `binarize` - binarize a given input image
* `filter` - apply a chosen filter on a given image
* `lines` - find lines in a given image
* `blobs` - run blob detection on a given image
* `corners` - run corner detection on the input image
* `webcam` - open up a webcam viewer
* `flow` - run optical flow demo with webcam
* `phash` - compare the phash'es of two images
* `rotate` - rotate an image 90 degrees left or right

More instructions will be shown about these functions if you run them without any additional parameters.
