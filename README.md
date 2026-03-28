# CLI Image Editor

This project is a command-line image editor developed in C that allows loading, processing, and saving images in Netpbm formats (PGM, PPM). The program supports both ASCII and binary formats and provides a suite of commands for pixel manipulation using efficient dynamic memory allocation.

## Architecture and Data Structures

For efficient and modular data management, the program utilizes the following core structures:

* **`image_t`**: The core structure holding the current state of the image. It stores the image type (RGB or Grayscale), dimensions (width and height), maximum intensity, dynamically allocated matrix pointers, and the current selection.
* **`colors_t`**: Represents a single pixel in a color image, storing the Red, Green, and Blue channel values.
* **`coords_t` & `rotation_limits_t`**: Manages the coordinates of the active selection area. They allow applying filters and transformations strictly to a specific subset of pixels.

## Features and Commands

The interactive interface supports a series of basic and advanced commands for image editing:

* **`LOAD <file>`**
    Parses the source file and loads the image into memory. If an image is already loaded, the old memory is automatically freed.
* **`SELECT <x1> <y1> <x2> <y2>`** / **`SELECT ALL`**
    Defines a region of interest. All subsequent effects and transformations will be applied strictly within these boundaries.
* **`ROTATE <angle>`**
    Rotates the current selection by angles divisible by 90° (both positive and negative). The transformation is done in-place for square selections or by reallocation for the entire image.
* **`CROP`**
    Crops the image to the current selection dimensions. Pixels outside the selection are discarded, and the memory is resized accordingly.
* **`EQUALIZE`**
    Improves the contrast of Grayscale images by applying a histogram equalization algorithm. It uses a clamping function to ensure the integrity of the pixel values.
* **`APPLY <filter>`**
    Applies convolution matrix transformations (3x3 kernels) to the selection in RGB images. Supported filters are: `EDGE`, `SHARPEN`, `BLUR`, and `GAUSSIAN_BLUR`.
* **`SAVE <file> [ascii]`**
    Exports the current image state to disk. It allows flexible conversion between binary (default) and text (ASCII) formats using the optional flag.
* **`EXIT`**
    Safely closes the program, ensuring the complete release of all dynamically allocated resources to prevent memory leaks.

## File Parsing and Memory Management

Pixel matrices are managed entirely through dynamic allocation. The parsing engine reads Netpbm files as follows:
1. Identifies the Magic Word (`P1`-`P6`) to determine the format and image type (ASCII/Binary, Grayscale/RGB).
2. Automatically skips comments (lines starting with `#`).
3. Extracts the image dimensions and maximum color intensity.
4. Populates the internal `image_t` matrix, preparing the environment for the editing workflow.