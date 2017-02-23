#ifndef NDEBUG
#include <iostream>
#include <fstream>
#endif
#include <queue>

#include "kd_tree.hpp"

#ifndef NDEBUG
using std::ofstream;
using std::ios;
using std::endl;
#endif

treeNode::treeNode(Photon p, superKey plane)
{
  photon = p;
  haveChilds = false;
  leftChild = NULL;
  rightChild = NULL;
  cutPlane = plane;
}

treeNode::~treeNode()
{
  haveChilds = false;
  if(leftChild) delete leftChild;
  if(rightChild) delete rightChild;
  leftChild = NULL;
  rightChild = NULL;
}

inline bool treeNode::operator==(treeNode b)
{
  return photon == b.photon;
}

inline bool treeNode::operator==(Photon p)
{
  return photon == p;
}

inline bool treeNode::operator>(treeNode b)
{
  return photon.greater(b.photon, cutPlane);
}

inline bool treeNode::operator>(Photon p)
{
  return photon.greater(p, cutPlane);
}

inline bool treeNode::operator<(treeNode b)
{
  return photon.lower(b.photon, cutPlane);
}

inline bool treeNode::operator<(Photon p)
{
  return photon.lower(p, cutPlane);
}

inline bool treeNode::operator>=(Vec3 p)
{
  return (cutPlane == XYZ && photon.position.x >= p.x) || 
			       (cutPlane == YZX && photon.position.y >= p.y) || 
    (cutPlane == ZXY && photon.position.z >= p.z);
}

inline bool treeNode::operator<=(Vec3 p)
{
  return (cutPlane == XYZ && photon.position.x <= p.x) || 
    (cutPlane == YZX && photon.position.y <= p.y) || 
    (cutPlane == ZXY && photon.position.z <= p.z);
}

template <class T>
void copyArray(T a[], int size, T b[])
{
  for(int i = 0; i < size; i++)
    {
      b[i] = a[i];
    }
}

template <class T>
void restoreArray(T a, int size)
{
  for(int i = 0; i < size; i++)
    a[i] = i;
}

template <class T>
void Merge(T a[], int begin, int middle, int end, T b[], superKey key, std::vector<Photon> originalData)
{
  int i = begin, j = middle;

  for(int k = begin; k < end; k++)
    {
      if(i < middle && (j >= end || originalData[a[i]].lessEqual(originalData[a[j]], key)))
	{
	  b[k] = a[i];
	  i++;
	}
      else
	{
	  b[k] = a[j];
	  j++;
	}
    }
}

template <class T>
void SplitMerge(T b[], int begin, int end, T a[], superKey key, std::vector<Photon> originalData)
{
  if(end - begin < 2)
    return;

  int middle = (begin + end) / 2;

  SplitMerge(a, begin, middle, b, key, originalData);
  SplitMerge(a, middle, end, b, key, originalData);

  Merge(b, begin, middle, end, a, key, originalData);
}

template <class T>
void MegeSort(T a[], T b[], int size, superKey key, std::vector<Photon> originalData)
{
  SplitMerge(b, 0, size, a, key ,originalData);
}

kdTree::kdTree(){root = NULL;}
kdTree::~kdTree(){}

void kdTree::addPhoton(Photon p)
{
  
  Photons.push_back(p);
}


void kdTree::createNodeKdTree(treeNode** node, std::vector<Photon> & originalData , int* xyz, int* yzx, int* zxy, superKey key, int begin, int end, int* xyz_2, int* yzx_2, int* zxy_2)
{
  if(end - begin < 2)
    {
      switch(key)
	{
	case XYZ:
	  *node = new treeNode(originalData[xyz[begin]], XYZ);
	  //std::cout << "Photon posicion " << xyz[begin] << " " << originalData[xyz[begin]] << std::endl;
	  break;
	case YZX:
	  *node = new treeNode(originalData[yzx[begin]], YZX);
	  //std::cout << "Photon posicion " << yzx[begin] << " " << originalData[yzx[begin]] << std::endl;
	  break;
	case ZXY:
	  *node = new treeNode(originalData[zxy[begin]], ZXY);
	  //std::cout << "Photon posicion " << zxy[begin] << " " << originalData[zxy[begin]] << std::endl;
	  break;
	}

      return;
    }

  int mid = (begin + end) / 2;
	
  switch(key)
    {
    case XYZ:
      *node = new treeNode(originalData[xyz[mid]], XYZ);
      (*node)->setChildsFlag(true);
      //std::cout << "Photon posicion " << xyz[mid] << " " << originalData[xyz[mid]] << std::endl;
      reorderArrays(originalData, yzx, zxy, begin, mid, end, xyz[mid], XYZ, yzx_2, zxy_2);
      key = YZX;
      break;
    case YZX:
      *node = new treeNode(originalData[yzx[mid]], YZX);
      (*node)->setChildsFlag(true);
      //std::cout << "Photon posicion " << yzx[mid] << " " << originalData[yzx[mid]] << std::endl;
      reorderArrays(originalData, xyz, zxy, begin, mid, end, yzx[mid], YZX, xyz_2, zxy_2);
      key = ZXY;
      break;
    case ZXY:
      *node = new treeNode(originalData[zxy[mid]], ZXY);
      (*node)->setChildsFlag(true);
      //std::cout << "Photon posicion " << zxy[mid] << " " << originalData[zxy[mid]] << std::endl;
      reorderArrays(originalData, xyz, yzx, begin, mid, end, zxy[mid], ZXY, xyz_2, yzx_2);
      key = XYZ;
      break;
    }

  //std::cout<<"Rama izquierda" << std::endl;
  createNodeKdTree((*node)->getLeftChild(), originalData, xyz_2, yzx_2, zxy_2, key, begin, mid, xyz, yzx, zxy);
  //std::cout<<"Rama derecha" << std::endl;
  createNodeKdTree((*node)->getRightChild(), originalData, xyz_2, yzx_2, zxy_2, key, mid + 1, end, xyz, yzx, zxy);
}

void kdTree::reorderArrays(std::vector<Photon> & originalData, int* A1, int* A2, int begin, int mid, int end, int orderIndex, superKey key, int* B1, int* B2)
{
  int lowerindex1 = begin, higherindex1 = mid + 1, lowerindex2 = begin, higherindex2 = mid + 1;

  for(int i = begin; i < end; i++)
    {
      if(A1[i] != orderIndex)
	{
	  if(originalData[A1[i]].lower(originalData[orderIndex], key))
	    {
	      B1[lowerindex1] = A1[i];
	      lowerindex1++;
	    }
	  else
	    {
	      B1[higherindex1] = A1[i];
	      higherindex1++;
	    }
	}

      if(A2[i] != orderIndex)
	{
	  if(originalData[A2[i]].lower(originalData[orderIndex], key))
	    {
	      B2[lowerindex2] = A2[i];
	      lowerindex2++;
	    }
	  else
	    {
	      B2[higherindex2] = A2[i];
	      higherindex2++;
	    }
	}
    }
}

bool kdTree::buildKdTree()
{	
  int size = Photons.size();
  //Arreglos con las superclaves de los photones
  int *xyz = new int[size];
  int *yzx = new int[size];
  int *zxy = new int[size];
  int *xyz_aux = new int[size];
  int *yzx_aux = new int[size];
  int *zxy_aux = new int[size];

  for(int i = 0; i < size; i++)
    {
      xyz[i] = i;
      yzx[i] = i;
      zxy[i] = i;
      xyz_aux[i] = i;
    }

  MegeSort(xyz, xyz_aux, size, XYZ, Photons);
  restoreArray(xyz_aux, size);

  MegeSort(yzx, xyz_aux, size, YZX, Photons);
  restoreArray(xyz_aux, size);

  MegeSort(zxy, xyz_aux, size, ZXY, Photons);

  for(int i = 0; i < size; i++)
    {
      xyz_aux[i] = xyz[i];
      yzx_aux[i] = yzx[i];
      zxy_aux[i] = zxy[i];
    }

  createNodeKdTree(&root, Photons , xyz, yzx, zxy, XYZ, 0, size, xyz_aux, yzx_aux, zxy_aux);

  //printTree();

#ifndef NDEBUG
  ofstream ofs("photons.txt", ios::out);
  float r, g, b;
  for (std::vector<Photon>::iterator it = Photons.begin(); it != Photons.end(); it++) {
    rgbe2float(r, g, b, (*it).radiance);
    ofs << (*it).position.x << " " << (*it).position.y << " " << (*it).position.z << " " << r << " " << g << " " << b << endl;
  }
  ofs.close();
#endif
  
  return true;
}

void kdTree::printTree(){
  std::queue<treeNode*> q;
  q.push(root);

  while(!q.empty())
    {
      treeNode* n = q.front();
      q.pop();
      if(n)
	{
	  std::cout << *n << " ";
			
	  if(q.empty() || !q.front() || *(q.front()->getCutPlane()) != *(n->getCutPlane()))
	    std::cout << std::endl;

	  q.push(*(n->getLeftChild()));
	  q.push(*(n->getRightChild()));
	}
    }
}

void kdTree::printNode(treeNode* node)
{
}

std::vector<Photon> kdTree::findInRange (Vec3 min, Vec3 max) const
{
  std::vector<Photon> photons;

  findInRange(min, max, photons, root);

  return photons;
}

void kdTree::findInRange (Vec3 min, Vec3 max, std::vector<Photon> &photons, treeNode *node) const
{	
  if(node == NULL) return;

  Vec3 position = node->getPhoton()->position;
  if(position >= min && position <= max)
    photons.push_back(*node->getPhoton());
	
  if(*node >= min)
    findInRange(min, max, photons, *node->getLeftChild());

  if(*node <= max)
    findInRange(min, max, photons, *node->getRightChild());

}
