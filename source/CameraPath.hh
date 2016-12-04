#ifndef __INF251_CODE_CAMERAPATH__90498145
#define __INF251_CODE_CAMERAPATH__90498145

#include "infdef.hh"

class CameraPath {
  std::vector<glm::vec3> mPosVertices;
  std::vector<glm::mat4x3> mPosSplines;

  std::vector<glm::vec3> mDirVertices;
  std::vector<glm::mat4x3> mDirSplines;
  GLuint mVbo {};
  GLuint mVao {};

  void init();

public:
  ~CameraPath();

  void add(glm::vec3 position, glm::vec3 direction = {});

  void buildSplines();

  // Returns a pair of (Position, Direction), in that order.
  // Direction is always normalised.
  std::pair<glm::vec3, glm::vec3> interp(float t) const;

  std::pair<glm::vec3, glm::vec3> operator[](float t) const
  { return interp(t); }

  void draw() const;
};

#endif //__INF251_CODE_CAMERAPATH__90498145
