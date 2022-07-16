/*
*  File:        ptree.cpp
*  Description: Implementation of a partitioning tree class for CPSC 221 PA3
*  Date:        2022-03-03 01:53
*
*               ADD YOUR PRIVATE FUNCTION IMPLEMENTATIONS TO THE BOTTOM OF THIS FILE
*/

#include "ptree.h"
#include "hue_utils.h" // useful functions for calculating hue averages

using namespace cs221util;
using namespace std;

// The following definition may be convenient, but is not necessary to use
typedef pair<unsigned int, unsigned int> pairUI;

/////////////////////////////////
// PTree private member functions
/////////////////////////////////

/*
*  Destroys all dynamically allocated memory associated with the current PTree object.
*  You may want to add a recursive helper function for this!
*  POST: all nodes allocated into the heap have been released.
*/
void PTree::Clear() {
  // add your implementation below
  if (root != nullptr) {
    Clear(root);
    root = nullptr;
  }
}

/*
*  Copies the parameter other PTree into the current PTree.
*  Does not free any memory. Should be called by copy constructor and operator=.
*  You may want a recursive helper function for this!
*  PARAM: other - the PTree which will be copied
*  PRE:   There is no dynamic memory associated with this PTree.
*  POST:  This PTree is a physically separate copy of the other PTree.
*/
void PTree::Copy(const PTree& other) {
  root = new Node(other.root->upperleft,other.root->width,other.root->height,other.root->avg,NULL,NULL);

  root->A = BuildNodeCopy(other.root->A);
  root->B = BuildNodeCopy(other.root->B);
}

/*
*  Private helper function for the constructor. Recursively builds the tree
*  according to the specification of the constructor.
*  You *may* change this if you like, but we have provided here what we
*  believe will be sufficient to use as-is.
*  PARAM:  im - full reference image used for construction
*  PARAM:  ul - upper-left image coordinate of the currently building Node's image region
*  PARAM:  w - width of the currently building Node's image region
*  PARAM:  h - height of the currently building Node's image region
*  RETURN: pointer to the fully constructed Node
*/
Node* PTree::BuildNode(PNG& im, pair<unsigned int, unsigned int> ul, unsigned int w, unsigned int h) {

  Node *node = new Node(ul,w,h,HSLAPixel(0,0,0,0),nullptr,nullptr);

  if (w == 1 && h == 1) { 
    HSLAPixel *pixel = im.getPixel(ul.first, ul.second);
    HSLAPixel colour = HSLAPixel(pixel->h,pixel->s,pixel->l,pixel->a);
    node->avg = colour;
    return node; 
    }

  HSLAPixel avgc = averageColor(im, ul, w, h);
  node->avg = avgc;

  if (w >= h) {
    int aWidth = w / 2;
    int bWidth = w - aWidth;
    node->A = BuildNode(im,ul,aWidth,h);
    node->B = BuildNode(im,make_pair(ul.first + aWidth,ul.second),bWidth,h);

  } else {
    int aHeight = h / 2;
    int bHeight = h - aHeight;
    node->A = BuildNode(im,ul,w,aHeight);
    node->B = BuildNode(im,make_pair(ul.first,ul.second + aHeight),w,bHeight);
  }
  return node;
}


////////////////////////////////
// PTree public member functions
////////////////////////////////

/*
*  Constructor that builds the PTree using the provided PNG.
*
*  The PTree represents the sub-image (actually the entire image) from (0,0) to (w-1, h-1) where
*  w-1 and h-1 are the largest valid image coordinates of the original PNG.
*  Each node corresponds to a rectangle of pixels in the original PNG, represented by
*  an (x,y) pair for the upper-left corner of the rectangle, and two unsigned integers for the
*  number of pixels on the width and height dimensions of the rectangular sub-image region the
*  node defines.
*
*  A node's two children correspond to a partition of the node's rectangular region into two
*  equal (or approximately equal) size regions which are either tiled horizontally or vertically.
*
*  If the rectangular region of a node is taller than it is wide, then its two children will divide
*  the region into vertically-tiled sub-regions of equal height:
*  +-------+
*  |   A   |
*  |       |
*  +-------+
*  |   B   |
*  |       |
*  +-------+
*
*  If the rectangular region of a node is wider than it is tall, OR if the region is exactly square,
*  then its two children will divide the region into horizontally-tiled sub-regions of equal width:
*  +-------+-------+
*  |   A   |   B   |
*  |       |       |
*  +-------+-------+
*
*  If any region cannot be divided exactly evenly (e.g. a horizontal division of odd width), then
*  child B will receive the larger half of the two subregions.
*
*  When the tree is fully constructed, each leaf corresponds to a single pixel in the PNG image.
* 
*  For the average colour, this MUST be computed separately over the node's rectangular region.
*  Do NOT simply compute this as a weighted average of the children's averages.
*  The functions defined in hue_utils.h and implemented in hue_utils.cpp will be very useful.
*  Computing the average over many overlapping rectangular regions sounds like it will be
*  inefficient, but as an exercise in theory, think about the asymptotic upper bound on the
*  number of times any given pixel is included in an average calculation.
* 
*  PARAM: im - reference image which will provide pixel data for the constructed tree's leaves
*  POST:  The newly constructed tree contains the PNG's pixel data in each leaf node.
*/
PTree::PTree(PNG& im) {
  root = BuildNode(im, make_pair(0,0), im.width(), im.height());
}

/*
*  Copy constructor
*  Builds a new tree as a copy of another tree.
*
*  PARAM: other - an existing PTree to be copied
*  POST:  This tree is constructed as a physically separate copy of other tree.
*/
PTree::PTree(const PTree& other) {
  Copy(other);
}

/*
*  Assignment operator
*  Rebuilds this tree as a copy of another tree.
*
*  PARAM: other - an existing PTree to be copied
*  POST:  If other is a physically different tree in memory, all pre-existing dynamic
*           memory in this tree is deallocated and this tree is reconstructed as a
*           physically separate copy of other tree.
*         Otherwise, there is no change to this tree.
*/
PTree& PTree::operator=(const PTree& other) {
  // add your implementation below
  if (this != &other) {
    this->Clear();
    Copy(other);
  }
  return *this;
}

/*
*  Destructor
*  Deallocates all dynamic memory associated with the tree and destroys this PTree object.
*/
PTree::~PTree() {
  // add your implementation below
  this->Clear();
}

/*
*  Traverses the tree and puts the leaf nodes' color data into the nodes'
*  defined image regions on the output PNG.
*  For non-pruned trees, each leaf node corresponds to a single pixel that will be coloured.
*  For pruned trees, each leaf node may cover a larger rectangular region that will be
*  entirely coloured using the node's average colour attribute.
*
*  You may want to add a recursive helper function for this!
*
*  RETURN: A PNG image of appropriate dimensions and coloured using the tree's leaf node colour data
*/
PNG PTree::Render() const {
  PNG png = PNG(root->width,root->height);

  drawPNG(root, png);

  return png;
}

/*
*  Trims subtrees as high as possible in the tree. A subtree is pruned
*  (its children are cleared/deallocated) if ALL of its leaves have colour
*  within tolerance of the subtree root's average colour.
*  Pruning criteria should be evaluated on the original tree, and never on a pruned
*  tree (i.e. we expect that Prune would be called on any tree at most once).
*  When processing a subtree, you should determine if the subtree should be pruned,
*  and prune it if possible before determining if it has subtrees that can be pruned.
* 
*  You may want to add (a) recursive helper function(s) for this!
*
*  PRE:  This tree has not been previously pruned (and is not copied/assigned from a tree that has been pruned)
*  POST: Any subtrees (as close to the root as possible) whose leaves all have colour
*        within tolerance from the subtree's root colour will have their children deallocated;
*        Each pruned subtree's root becomes a leaf node.
*/
void PTree::Prune(double tolerance) {
  pruner(root, tolerance);
}

/*
*  Returns the total number of nodes in the tree.
*  This function should run in time linearly proportional to the size of the tree.
*
*  You may want to add a recursive helper function for this!
*/
int PTree::Size() const {
  // replace the line below with your implementation
  int size = 0;

  if (root != NULL) {
    size += subSize(root);
  }

  return size;
}


/*
*  Returns the total number of leaf nodes in the tree.
*  This function should run in time linearly proportional to the size of the tree.
*
*  You may want to add a recursive helper function for this!
*/
int PTree::NumLeaves() const {
  // replace the line below with your implementation
  int num = 0;

  if (root != NULL) {
    num += subLeaves(root);
  }

  return num;
}

/*
*  Rearranges the nodes in the tree, such that a rendered PNG will be flipped horizontally
*  (i.e. mirrored over a vertical axis).
*  This can be achieved by manipulation of the nodes' member attribute(s).
*  Note that this may possibly be executed on a pruned tree.
*  This function should run in time linearly proportional to the size of the tree.
*
*  You may want to add a recursive helper function for this!
*
*  POST: Tree has been modified so that a rendered PNG will be flipped horizontally.
*/
void PTree::FlipHorizontal() {
  flipHorizontal(root);
}

/*
*  Like the function above, rearranges the nodes in the tree, such that a rendered PNG
*  will be flipped vertically (i.e. mirrored over a horizontal axis).
*  This can be achieved by manipulation of the nodes' member attribute(s).
*  Note that this may possibly be executed on a pruned tree.
*  This function should run in time linearly proportional to the size of the tree.
*
*  You may want to add a recursive helper function for this!
*
*  POST: Tree has been modified so that a rendered PNG will be flipped vertically.
*/
void PTree::FlipVertical() {
  flipVertical(root);
}

/*
    *  Provides access to the root of the tree.
    *  Dangerous in practice! This is only used for testing.
    */
Node* PTree::GetRoot() {
  return root;
}

//////////////////////////////////////////////
// PERSONALLY DEFINED PRIVATE MEMBER FUNCTIONS
//////////////////////////////////////////////

void PTree::drawPNG(Node* node, PNG& png) const {
  if (node != nullptr) {
    if (node->A == nullptr && node->B == nullptr) {
      pairUI ul = node->upperleft;
      HSLAPixel nodePixel = node->avg;
      for (unsigned x = ul.first; x < ul.first + node->width; x++) {
        for (unsigned y = ul.second; y < ul.second + node->height; y++) {
          HSLAPixel *pngPixel = png.getPixel(x,y);

          pngPixel->h = nodePixel.h;
          pngPixel->s = nodePixel.s;
          pngPixel->l = nodePixel.l;
          pngPixel->a = nodePixel.a;
        }
      }
    }
      
    if (node->A != nullptr) { drawPNG(node->A, png); }
    if (node->B != nullptr) { drawPNG(node->B, png); }
  }
}

HSLAPixel PTree::averageColor(PNG& im, pair<unsigned int, unsigned int> ul, unsigned int w, unsigned int h) {
  double sumHX = 0;
  double sumHY = 0;
  double sumS = 0;
  double sumL = 0;
  double sumA = 0;
  
  double denominator = 0;

  for (unsigned x = ul.first; x < w + ul.first; x++) {
    for (unsigned y = ul.second; y < h + ul.second; y++) {
      HSLAPixel* pixel = im.getPixel(x, y);

      sumHX += Deg2X(pixel->h);
      sumHY += Deg2Y(pixel->h);
      sumS += pixel->s;
      sumL += pixel->l;
      sumA += pixel->a;
        
      denominator++;
    }
  }

  double avgHX = sumHX/denominator;
  double avgHY = sumHY/denominator;
  double avgH = XY2Deg(avgHX, avgHY);

  double avgS = sumS/denominator;
  double avgL = sumL/denominator;
  double avgA = sumA/denominator;

  HSLAPixel avg = HSLAPixel(avgH, avgS, avgL, avgA);

  return avg;
}

Node* PTree::copyNode(Node* node) {
  Node *copy = new Node(node->upperleft,node->width,node->height,node->avg,nullptr,nullptr);

  if (node->A == nullptr && node->B == nullptr) { return copy; }

  if (node->A != nullptr) { copy->A = copyNode(node->A); }
  if (node->B != nullptr) { copy->B = copyNode(node->B); }

  return copy;
}

void PTree::Clear(Node *subroot) {

  if(root != nullptr) {
    Clear(subroot->A);
    Clear(subroot->B);

    delete subroot;
    subroot = NULL;
  }
}

int PTree::subSize(Node* subroot) const {
  int size = 0;

  if (subroot == nullptr) { return size; }

  size++;

  if (subroot->A != nullptr) { size += subSize(subroot->A); }
  if (subroot->B != nullptr) { size += subSize(subroot->B); }

  return size;
}

int PTree::subLeaves(Node* subroot) const {
  int num = 0;

  if (subroot == nullptr) { return num; }

  if (subroot->A == nullptr && subroot->B == nullptr) { num++; }

  if (subroot->A != nullptr) { num += subLeaves(subroot->A); }
  if (subroot->B != nullptr) { num += subLeaves(subroot->B); }

  return num;
}

void PTree::pruner(Node* node, double tolerance) {
  if (node != nullptr) {
    if (prunable(node, node, tolerance)) {
      Clear(node->A);
      Clear(node->B);
      node->A = nullptr;
      node->B = nullptr;
    } else {
      pruner(node->A, tolerance);
      pruner(node->B, tolerance);
    }
  }
}

bool PTree::prunable(Node* original, Node* node, double tolerance) {
  if (node->A == nullptr && node->B == nullptr) {
    return original->avg.dist(node->avg) <= tolerance;
  } else {
    return prunable(original, node->A, tolerance) && prunable(original, node->B, tolerance);
  }
}

void PTree::flipHorizontal(Node* node) {
  PNG png(node->width,node->height);
  drawPNG(node,png);
  PNG pngCopy(png);

  for (size_t originalX = 0, mirrorX = node->width - 1; originalX < node->width; originalX++, mirrorX--) {
    for (size_t y = 0; y < node->height; y++) {
      HSLAPixel *original = png.getPixel(originalX,y);
      HSLAPixel *mirror = pngCopy.getPixel(mirrorX,y);

      mirror->h = original->h;
      mirror->s = original->s;
      mirror->l = original->l;
      mirror->a = original->a;
    }
  }

  Clear(root);
  root = BuildNode(pngCopy,make_pair(0,0),pngCopy.width(),pngCopy.height());
}

void PTree::flipVertical(Node* node) {
  PNG png(node->width,node->height);
  drawPNG(node,png);
  PNG pngCopy(png);

  for (size_t x = 0; x < node->width; x++) {
    for (size_t originalY = 0, mirrorY = node->height - 1; originalY < node->height; originalY++, mirrorY--) {
      HSLAPixel *original = png.getPixel(x,originalY);
      HSLAPixel *mirror = pngCopy.getPixel(x,mirrorY);

      mirror->h = original->h;
      mirror->s = original->s;
      mirror->l = original->l;
      mirror->a = original->a;
    }
  }

  Clear(root);
  root = BuildNode(pngCopy,make_pair(0,0),pngCopy.width(),pngCopy.height());
}