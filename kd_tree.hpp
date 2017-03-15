#include <vector>
#include <limits>
#include <iostream>
#include <glm/glm.hpp>

#include "rgbe.hpp"

enum superKey
  {
    XYZ,
    YZX,
    ZXY
  };

struct Vec3
{
  float x;
  float y;
  float z;

  Vec3(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f): x(_x), y(_y), z(_z) { }

  Vec3(const Vec3 & other) {
    x = other.x;
    y = other.y;
    z = other.z;
  }
  
  inline bool equalFloat(const float x, const float y)
  {
    return (x - std::numeric_limits<float>::epsilon() <= y) && (x + std::numeric_limits<float>::epsilon() >= y);
  }

  inline bool operator<=(const Vec3 b)
  {
    return (x < b.x || (equalFloat(x, b.x) && (y < b.y || (equalFloat(y, b.y) && z <= b.z))));
  }

  inline bool operator>=(const Vec3 b)
  {
    return (x > b.x || (equalFloat(x, b.x) && (y > b.y || (equalFloat(y, b.y) && z >= b.z))));
  }

  inline friend std::ostream& operator<<(std::ostream& out, const Vec3& v)
  {
    return out << "X:" << v.x << " Y:" << v.y << " Z:" << v.z;
  }
};

struct Photon
{
  Vec3 position;
  Vec3 direction;
  float ref_index;
  unsigned char radiance[4];
  float r, g, b;

  Photon(Vec3 _p = Vec3(), Vec3 _d = Vec3(), float red = 0.0f, float green = 0.0f, float blue = 0.0f, float _r = 1.0f):
    position(_p),
    direction(_d),
    ref_index(_r),
    r(red),
    g(green),
    b(blue)
  {
    float2rgbe(radiance, red, green, blue);
  }

  inline void getColor(float & red, float & green, float & blue) {
    rgbe2float(red, green, blue, radiance);
  }
  
  inline bool equalFloat(const float x, const float y)
  {
    return (x - std::numeric_limits<float>::epsilon() <= y) && (x + std::numeric_limits<float>::epsilon() >= y);
  }
  
  inline bool operator==(const Photon p)
  {
    return equalFloat(position.x, p.position.x) && equalFloat(position.y, p.position.y) && equalFloat(position.z, p.position.z);
  }

  inline bool lessEqual(const Photon& b, superKey key)
  {
    return (key == XYZ && (position.x < b.position.x || (equalFloat(position.x, b.position.x) && (position.y < b.position.y || (equalFloat(position.y, b.position.y) && position.z <= b.position.z)))))
					|| (key == YZX && (position.y < b.position.y || (equalFloat(position.y, b.position.y) && (position.z < b.position.z || (equalFloat(position.z, b.position.z) && position.x <= b.position.x)))))
									|| (key == ZXY && (position.z < b.position.z || (equalFloat(position.z, b.position.z) && (position.x < b.position.x || (equalFloat(position.x, b.position.x) && position.y <= b.position.y)))));
  }

  inline bool lower(const Photon &b, superKey key)
  {
    return (key == XYZ && (position.x < b.position.x || (equalFloat(position.x, b.position.x) && (position.y < b.position.y || (equalFloat(position.y, b.position.y) && position.z < b.position.z)))))
					|| (key == YZX && (position.y < b.position.y || (equalFloat(position.y, b.position.y) && (position.z < b.position.z || (equalFloat(position.z, b.position.z) && position.x < b.position.x)))))
									|| (key == ZXY && (position.z < b.position.z || (equalFloat(position.z, b.position.z) && (position.x < b.position.x || (equalFloat(position.x, b.position.x) && position.y < b.position.y)))));
  }

  inline bool greater(const Photon &b, superKey key)
  {
    return (key == XYZ && (position.x > b.position.x || (equalFloat(position.x, b.position.x) && (position.y > b.position.y || (equalFloat(position.y, b.position.y) && position.z > b.position.z)))))
									|| (key == YZX && (position.y > b.position.y || (equalFloat(position.y, b.position.y) && (position.z > b.position.z || (equalFloat(position.z, b.position.z) && position.x > b.position.x)))))
					|| (key == ZXY && (position.z > b.position.z || (equalFloat(position.z, b.position.z) && (position.x > b.position.x || (equalFloat(position.x, b.position.x) && position.y > b.position.y)))));
  }

  inline friend std::ostream& operator<<(std::ostream& out, const Photon& p)
  {
    return out << "Position: " << p.position;
  }
};

class treeNode
{
private:
  Photon photon;
  bool haveChilds;
  superKey cutPlane;
  treeNode* leftChild;
  treeNode* rightChild;
public:
  treeNode(Photon p, superKey plane);
  ~treeNode();
  inline bool operator==(treeNode b);
  inline bool operator>(treeNode b);
  inline bool operator<(treeNode b);
  inline bool operator==(Photon p);
  inline bool operator>(Photon p);
  inline bool operator<(Photon p);
  inline bool operator>=(Vec3 p);
  inline bool operator<=(Vec3 p);
  inline friend std::ostream& operator<<(std::ostream& out, treeNode &t)
  {	
    out << "Cut Plane ";
		
    switch(*(t.getCutPlane()))
      {
      case XYZ:
	out << "X ";
	break;
      case YZX:
	out << "Y ";
	break;
      case ZXY:
	out << "Z ";
	break;
      }
    return out << "Photon " << *(t.getPhoton()) << " HaveChilds: " << *(t.getChildsFlag());		
  }

  Photon* getPhoton(){return &photon;}
  bool* getChildsFlag(){return &haveChilds;}
  void setChildsFlag(bool childs){haveChilds = childs;}
  superKey* getCutPlane(){return &cutPlane;}
  treeNode** getLeftChild(){return &leftChild;}
  treeNode** getRightChild(){return &rightChild;}
};

//Clase que genera el kdTree
class kdTree
{
public:
  kdTree();
  ~kdTree();

  void addPhoton(Photon p);
  bool buildKdTree();
  void printTree();
  std::vector<Photon> findInRange (Vec3 min, Vec3 max) const;
  void find_by_distance(std::vector<Photon> & found, const glm::vec3 & point, const glm::vec3 & normal, const float distance, const unsigned int max) const;
  void save_photon_list(const char * file_name) const;
  size_t getNumPhotons();
  
private:
  treeNode* root;
  std::vector<Photon> Photons;

  void createNodeKdTree(treeNode** node,
			std::vector<Photon> & originalData ,
			int* xyz,
			int* yzx,
			int* zxy,
			superKey key,
			int begin,
			int end,
			int* xyz_2,
			int* yzx_2,
			int* zxy_2);

  void reorderArrays(std::vector<Photon> & originalData,
		     int* A1,
		     int* A2,
		     int begin,
		     int mid,
		     int end,
		     int orderIndex,
		     superKey Key,
		     int* B1,
		     int* B2);

  void printNode(treeNode* node);

  void findInRange (Vec3 min, Vec3 max, std::vector<Photon> & photons, treeNode *node) const;
};
