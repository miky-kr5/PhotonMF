#include <iostream>
#include <fstream>
#include <queue>

#include <omp.h>
#include <glm/gtc/constants.hpp>

#include "kd_tree.hpp"

using std::ofstream;
using std::ios;
using std::cout;
using std::endl;

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
#pragma omp parallel for schedule(dynamic, 1)
  for(int i = 0; i < size; i++)
    a[i] = i;
}

template <class T>
void Merge(T a[], int begin, int middle, int end, T b[], superKey key, std::vector<Photon> & originalData)
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
void SplitMerge(T b[], int begin, int end, T a[], superKey key, std::vector<Photon> & originalData)
{
  if(end - begin < 2)
    return;

  int middle = (begin + end) / 2;

  SplitMerge(a, begin, middle, b, key, originalData);
  SplitMerge(a, middle, end, b, key, originalData);

  Merge(b, begin, middle, end, a, key, originalData);
}

template <class T>
void MegeSort(T a[], T b[], int size, superKey key, std::vector<Photon> & originalData)
{
  SplitMerge(b, 0, size, a, key, originalData);
}

kdTree::kdTree(){root = NULL;}
kdTree::~kdTree(){}

void kdTree::addPhoton(Photon p)
{
  Photons.push_back(p);
}

void kdTree::createNodeKdTree(treeNode** node, std::vector<Photon> originalData , int xyz[], int yzx[], int zxy[], superKey key, int begin, int end, int fullsize)
{

  int size = end - begin;

  if(size <= 2)
  {
    if(size < 2)
    {
      switch(key)
      {
        case XYZ:
          *node = new treeNode(originalData[xyz[begin]], XYZ);
          if(size == 1)
          {
            (*node)->setChildsFlag(true);
            *((*node)->getRightChild()) = new treeNode(originalData[xyz[end]], YZX);
          }
        break;
        case YZX:
          *node = new treeNode(originalData[yzx[begin]], YZX);
          if(size == 1)
          {
            (*node)->setChildsFlag(true);
            *((*node)->getRightChild()) = new treeNode(originalData[yzx[end]], ZXY);
          }
        break;
        case ZXY:
          *node = new treeNode(originalData[zxy[begin]], ZXY);
          if(size == 1)
          {
            (*node)->setChildsFlag(true);
            *((*node)->getRightChild()) = new treeNode(originalData[zxy[end]], XYZ);
          }
        break;
      }
    }
    else
    {
      int mid = (begin + end) / 2;

      switch(key)
      {
        case XYZ:
          *node = new treeNode(originalData[xyz[mid]], XYZ);
          *((*node)->getLeftChild()) = new treeNode(originalData[xyz[begin]], YZX);
          *((*node)->getRightChild()) = new treeNode(originalData[xyz[end]], YZX);
        break;
        case YZX:
          *node = new treeNode(originalData[yzx[mid]], YZX);
          *((*node)->getLeftChild()) = new treeNode(originalData[yzx[begin]], ZXY);
          *((*node)->getRightChild()) = new treeNode(originalData[yzx[end]], ZXY);
        break;
        case ZXY:
          *node = new treeNode(originalData[zxy[mid]], ZXY);
          *((*node)->getLeftChild()) = new treeNode(originalData[zxy[begin]], XYZ);
          *((*node)->getRightChild()) = new treeNode(originalData[zxy[end]], XYZ);
        break;
      }
    }

    return;
  }

  int mid = (begin + end) / 2;
  
  switch(key)
  {
    case XYZ:
      *node = new treeNode(originalData[xyz[mid]], XYZ);
      (*node)->setChildsFlag(true);
      //std::cout << "CUT XYZ: " << xyz[mid] << std::endl;
      //std::cout << "Photon posicion " << xyz[mid] << " " << originalData[xyz[mid]] << std::endl;
      reorderArrays(originalData, yzx, zxy, begin, mid, end, xyz[mid], XYZ);
      xyz[mid] = -1;
      key = YZX;
      break;
    case YZX:
      *node = new treeNode(originalData[yzx[mid]], YZX);
      (*node)->setChildsFlag(true);
      //std::cout << "CUT YZX: " << yzx[mid] << std::endl;
      //std::cout << "Photon posicion " << yzx[mid] << " " << originalData[yzx[mid]] << std::endl;
      reorderArrays(originalData, xyz, zxy, begin, mid, end, yzx[mid], YZX);
      yzx[mid] = -1;
      key = ZXY;
      break;
    case ZXY:
      *node = new treeNode(originalData[zxy[mid]], ZXY);
      (*node)->setChildsFlag(true);
      //std::cout << "CUT ZXY: " << zxy[mid] << std::endl;
      //std::cout << "Photon posicion " << zxy[mid] << " " << originalData[zxy[mid]] << std::endl;
      reorderArrays(originalData, xyz, yzx, begin, mid, end, zxy[mid], ZXY);
      zxy[mid] = -1;
      key = XYZ;
      break;
  }

  /*std::cout << "XYZ: ";
  for(int i = 0; i < fullsize; i++)
    std::cout << xyz[i]  << " ";

  std::cout << std::endl << "YZX: ";
  for(int i = 0; i < fullsize; i++)
    std::cout << yzx[i]  << " ";
  
  std::cout << std::endl << "ZXY: ";
  for(int i = 0; i < fullsize; i++)
    std::cout << zxy[i]  << " ";

  std::cout << std::endl << std::endl;*/

  //std::cout<<"Rama izquierda" << std::endl;
  createNodeKdTree((*node)->getLeftChild(), originalData, xyz, yzx, zxy, key, begin, mid - 1, fullsize);
  //std::cout<<"Rama derecha" << std::endl;
  createNodeKdTree((*node)->getRightChild(), originalData, xyz, yzx, zxy, key, mid + 1, end, fullsize);
}

void kdTree::reorderArrays(std::vector<Photon> originalData, int A1[], int A2[], int begin, int mid, int end, int orderIndex, superKey key)
{
  std::vector<int> lower1, lower2, higher1, higher2;

  for(int i = begin; i <= end; i++)
  {   
    if(A1[i] != orderIndex)
    {
      if(originalData[A1[i]].lessEqual(originalData[orderIndex], key))
        lower1.push_back(A1[i]);
      else
        higher1.push_back(A1[i]);
    }

    if(A2[i] != orderIndex)
    {
      if(originalData[A2[i]].lessEqual(originalData[orderIndex], key))
        lower2.push_back(A2[i]);
      else
        higher2.push_back(A2[i]);
    }
  }

  std::copy(lower1.begin(), lower1.end(), &A1[begin]);
  std::copy(higher1.begin(), higher1.end(), &A1[mid + 1]);

  std::copy(lower2.begin(), lower2.end(), &A2[begin]);
  std::copy(higher2.begin(), higher2.end(), &A2[mid + 1]);
  
  A1[mid] = A2[mid] = -1;

  lower1.clear();
  lower2.clear();
  higher1.clear();
  higher2.clear();
}

void kdTree::removeDuplicates(std::vector<Photon> &originalData, int* &xyz, int* &yzx, int* &zxy, int &size)
{
  if(size == 0) return;

  std::vector<int> xyzNew;
  std::vector<int> yzxNew;
  std::vector<int> zxyNew;

  xyzNew.push_back(xyz[0]);
  yzxNew.push_back(yzx[0]);
  zxyNew.push_back(zxy[0]);

  for(int i = 1; i < size; i++)
  {
    if( !(originalData[xyz[i]] == originalData[xyz[i - 1]]) )
      xyzNew.push_back(xyz[i]);
    
    if( !(originalData[yzx[i]] == originalData[yzx[i - 1]]) )
      yzxNew.push_back(yzx[i]);
    
    if( !(originalData[zxy[i]] == originalData[zxy[i - 1]]) )
      zxyNew.push_back(zxy[i]);
  }

  if(xyzNew.size() != size)
  {
    std::cout << size - xyzNew.size() << " duplicates removed" << std::endl;

    delete []xyz;
    delete []yzx;
    delete []zxy;

    size = xyzNew.size();

    xyz = new int[size];
    yzx = new int[size];
    zxy = new int[size];

    std::copy(xyzNew.begin(), xyzNew.end(), &xyz[0]);
    std::copy(yzxNew.begin(), yzxNew.end(), &yzx[0]);
    std::copy(zxyNew.begin(), zxyNew.end(), &zxy[0]);
  }

  xyzNew.clear();
  yzxNew.clear();
  zxyNew.clear();
}

bool kdTree::buildKdTree()
{	
  int size = Photons.size();
  //Arreglos con las superclaves de los photones
  int *xyz = new int[size];
  int *yzx = new int[size];
  int *zxy = new int[size];

  int *aux = new int[size];

  for(int i = 0; i < size; i++)
  {
    xyz[i] = i;
    yzx[i] = i;
    zxy[i] = i;
    aux[i] = i;
  }

  std::cout << "Initial sorting of data" << std::endl;

  MegeSort(xyz, aux, size, XYZ, Photons);
  restoreArray(aux, size);

  MegeSort(yzx, aux, size, YZX, Photons);
  restoreArray(aux, size);

  MegeSort(zxy, aux, size, ZXY, Photons);

  std::cout << "Removing Doubles" << std::endl;
  removeDuplicates(Photons, xyz, yzx, zxy, size);

  createNodeKdTree(&root, Photons , xyz, yzx, zxy, XYZ, 0, size - 1, size);

  delete []xyz;
  delete []yzx;
  delete []zxy;
  delete []aux;
 
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

size_t kdTree::getNumPhotons() {
  return Photons.size();
}

void kdTree::save_photon_list(const char * file_name) const {
  cout << "Writing photons to \x1b[1;33m" << file_name << "\x1b[m" << endl;
  ofstream ofs(file_name, ios::out);
  float r, g, b;
  for (std::vector<Photon>::const_iterator it = Photons.begin(); it != Photons.end(); it++) {
    rgbe2float(r, g, b, (*it).radiance);
    ofs << (*it).position.x << " " << (*it).position.y << " " << (*it).position.z << " " <<
      (*it).direction.x << " " << (*it).direction.y << " " << (*it).direction.z << " " <<
      (*it).r << " " << (*it).g << " " << (*it).b << " " << (*it).ref_index << endl;
  }
  ofs.close();
}

void kdTree::find_by_distance(std::vector<Photon> & found, const glm::vec3 & point, const glm::vec3 & normal, const float distance, const unsigned int max) const {
  glm::vec3 p_pos;
  found.clear();
  for (std::vector<Photon>::const_iterator it = Photons.begin(); it != Photons.end(); it++) {
    p_pos = glm::vec3((*it).position.x, (*it).position.y, (*it).position.z);
    if (glm::distance(p_pos, point) < distance && glm::dot(glm::normalize(p_pos - point), normal) < glm::pi<float>() / 2.0f)
      found.push_back((*it));
    // if (found.size() >= max)
    //   break;
  }
}
