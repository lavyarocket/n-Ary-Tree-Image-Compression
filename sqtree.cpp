/**
 *
 * shifty quadtree (pa3)
 * sqtree.cpp
 * This file will be used for grading.
 *
 */

#include "sqtree.h"

// First Node constructor, given.
SQtree::Node::Node(pair<int,int> ul, int w, int h, RGBAPixel a, double v)
  :upLeft(ul),width(w),height(h),avg(a),var(v),NW(NULL),NE(NULL),SE(NULL),SW(NULL)
{}

// Second Node constructor, given
SQtree::Node::Node(stats & s, pair<int,int> ul, int w, int h)
  :upLeft(ul),width(w),height(h),NW(NULL),NE(NULL),SE(NULL),SW(NULL) {
  avg = s.getAvg(ul,w,h);
  var = s.getVar(ul,w,h);
}

// SQtree destructor, given.
SQtree::~SQtree() {
  clear();
}

// SQtree copy constructor, given.
SQtree::SQtree(const SQtree & temp) {
  copy(temp);
}

// SQtree assignment operator, given.
SQtree & SQtree::operator=(const SQtree & rhs) {
  if (this != &rhs) {
    clear();
    copy(rhs);
  }
  return *this;
}

/**
 * SQtree constructor given tolerance for variance.
 */
SQtree::SQtree(PNG & imIn, double tol) {
  
  pair<int,int> upper_left(0,0);
  stats stat(imIn);
  root = buildTree(stat, upper_left, imIn.width(), imIn.height(), tol);

}

/**
 * Helper for the SQtree constructor.
 */
SQtree::Node * SQtree::buildTree(stats & s, pair<int,int> & ul,
				 int w, int h, double tol) {

  Node *current = new Node(ul, w, h, s.getAvg(ul, w, h), s.getVar(ul, w, h));
  if ((h == 1 && w == 1) || current->var <= tol) {
    return current;
  }

  pair<int,int> break_point = ul;
  double var_arr[4] = {__DBL_MAX__, __DBL_MAX__, __DBL_MAX__, __DBL_MAX__};
  double max_var_arr = __DBL_MAX__;
  double temp_var_arr[4] = {__DBL_MIN__, __DBL_MIN__, __DBL_MIN__, __DBL_MIN__};
  double max_temp_var_arr;

  for (int i = 0; i < w; i++) {
    for (int j = 0; j < h; j++) {

      if (i == 0 && j == 0) {
        continue;
      }

      if (j == 0) {
      temp_var_arr[0] = s.getVar(ul, i, h);
      temp_var_arr[1] = s.getVar(make_pair(ul.first+i, ul.second), w-i, h);       
      } 
      else if (i == 0) {
      temp_var_arr[0] = s.getVar(ul, w, j);
      temp_var_arr[1] = s.getVar(make_pair(ul.first, ul.second+j), w, h-j);
      } 
      else { 
      temp_var_arr[0] = s.getVar(ul, i, j);
      temp_var_arr[1] = s.getVar(make_pair(ul.first+i, ul.second), w-i, j);
      temp_var_arr[2] = s.getVar(make_pair(ul.first+i, ul.second+j), w-i, h-j);
      temp_var_arr[3] = s.getVar(make_pair(ul.first, ul.second+j), i, h-j);
      }

      max_temp_var_arr = max(temp_var_arr[0], temp_var_arr[1]);
      max_temp_var_arr = max(max_temp_var_arr, temp_var_arr[2]);
      max_temp_var_arr = max(max_temp_var_arr, temp_var_arr[3]);

      if (max_temp_var_arr <= max_var_arr) {
      break_point = make_pair(ul.first+i, ul.second+j);
      var_arr[0] = temp_var_arr[0];
      var_arr[1] = temp_var_arr[1];
      var_arr[2] = temp_var_arr[2];
      var_arr[3] = temp_var_arr[3];
      max_var_arr = max(var_arr[0], var_arr[1]);
      max_var_arr = max(max_var_arr, var_arr[2]);
      max_var_arr = max(max_var_arr, var_arr[3]);
      }
    }
  }

  if (break_point.second == ul.second) {
    pair<int,int> ne(break_point.first, break_point.second);
    pair<int,int> nw(ul.first, ul.second);
    current->NE = buildTree(s, ne, ul.first + w - break_point.first, h, tol);
    current->NW = buildTree(s, nw, break_point.first - ul.first, h, tol);
  } 
  else if (break_point.first == ul.first) {
    pair<int,int> sw(break_point.first, break_point.second);
    pair<int,int> nw(ul.first, ul.second);
    current->SW = buildTree(s, sw, w, ul.second + h - break_point.second, tol);
    current->NW = buildTree(s, nw, w, break_point.second - ul.second, tol);
  } 
  else {
    pair<int,int> sw(ul.first, break_point.second);
    pair<int,int> ne(break_point.first, ul.second);
    pair<int,int> se(break_point.first, break_point.second);
    pair<int,int> nw(ul.first, ul.second);
    current->SW = buildTree(s, sw, break_point.first - ul.first, ul.second + h - break_point.second, tol);
    current->NE = buildTree(s, ne, ul.first + w - break_point.first, break_point.second - ul.second, tol);
    current->SE = buildTree(s, se, ul.first + w - break_point.first, ul.second + h - break_point.second, tol);
    current->NW = buildTree(s, nw, break_point.first - ul.first, break_point.second - ul.second, tol);
  }

  return current;

}
  
/**
 * Render SQtree and return the resulting image.
 */
PNG SQtree::render() {
  PNG im(root->width, root->height);
  render_helper(root, im);
  return im;
}

void SQtree::render_helper(Node *sub, PNG & im) {
  if (!sub) {
    return;
  }

  if (!(sub->SE || sub->NE || sub->NW || sub->SW)) {
    for (int i = 0; i < sub->height; i++) {
      for (int j = 0; j < sub->width; j++) {
        RGBAPixel *current = im.getPixel(sub->upLeft.first + j, sub->upLeft.second + i);
        *current = sub->avg;
      }
    }
  }
  render_helper(sub->SW, im);
  render_helper(sub->NE, im);
  render_helper(sub->NW, im);
  render_helper(sub->SE, im);
}

/**
 * Delete allocated memory.
 */
void SQtree::clear() {
  clear_helper(root);
}

void SQtree::clear_helper(Node *sub) {
  if (!sub) {
    return;
  }
  clear_helper(sub->SW);
  clear_helper(sub->NE);
  clear_helper(sub->NW);
  clear_helper(sub->SE);
  delete sub;
}

void SQtree::copy(const SQtree & temp) {
  root = copy_helper(root, temp.root);
}

SQtree::Node * SQtree::copy_helper(Node *sub, const Node *temp) {
  if (!temp) {
    return NULL;
  }
  sub = new Node(temp->upLeft, temp->width, temp->height, temp->avg, temp->var);
  sub->SW = copy_helper(sub->SW, temp->SW);
  sub->NE = copy_helper(sub->NE, temp->NE);
  sub->NW = copy_helper(sub->NW, temp->NW);
  sub->SE = copy_helper(sub->SE, temp->SE);
  return sub;
}

int SQtree::size() {
  return size_helper(root);
}

int SQtree::size_helper(Node *sub) {
  if (!sub) {
    return 0;
  }
  int size = size_helper(sub->SW) + size_helper(sub->NE) + size_helper(sub->NW) + size_helper(sub->SE) + 1;
  return size;
}

