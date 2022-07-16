# Image-Partitioning-Trees
- learn about representing images using a binary tree
- learn to design more complex recursive algorithms
- learn about a possible method of image compression using space-partitioning trees

# Part 1: The PTree Class
A PTree is a binary tree whose nodes represent rectangular regions of a PNG image. The root represents the entire image. The two children (referred to as A and B), of any node nd represent two equal or roughly equal horizontal or vertical partitions of nd's image region. Every node also contains:

- a pixel representing the average colour of the pixels in its rectangular region
- the upper-left image coordinates of its rectangular region
- the width of its rectangular region in pixels, and
- the height of its rectangular region in pixels

# Building a PTree
The contructor of the PTree receives a PNG image of non-zero dimensions.

The region for each node is split as equally as possible, and symmetrically, into two smaller regions along the longer of the two dimensions of the node being split. See the documentation in ptree.h for details about how to determine the split axis and how to determine the dimensions and coordinates of the split pieces. A freshly constructed PTree will have a leaf node corresponding to each individual pixel of the original image.

# Part 2: Image compression using PTrees
As a result of the hierarchical structure of the tree, along with each node storing the average colour of its rectangular region, we can trim away portions of the tree representing areas without fine pixel-level detail. This is achieved by the Prune function, which receives a tolerance parameter. This function attempts, starting near the top of a freshly built tree, to remove all of the descendants of a node, if all of the leaf nodes below the current node have colour within tolerance of the node's average colour.

In this way, areas of the image with little colour variability can be replaced with a single rectangle of solid colour, while areas with fine pixel detail can remain detailed. The image quality may be reduced, but fine details will still be visible, and the size of the structure used to store the image data will also be reduced.
