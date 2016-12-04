#include "CameraPath.hh"

namespace {
  glm::vec4 _hermite(float a, float b, float c, float d)
  {
    glm::vec4 v;

    v.x = -a / 2.0f + (3.0f * b)/2.0f - (3.0f * c)/2.0f + d/2.0f;
    v.y = a - (5.0f * b)/2.0f + 2.0f * c - d / 2.0f;
    v.z = -a / 2.0f + c / 2.0f;
    v.w = b;

    return v;
  }

  constexpr auto SPLINE_STEPS = 10;
}

CameraPath::~CameraPath()
{
  if (mVbo) {
    gl->glDeleteBuffers(1, &mVbo);
    gl->glDeleteVertexArrays(1, &mVao);
  }
}

void CameraPath::init()
{
  if (!mVbo) {
    gl->glGenBuffers(1, &mVbo);
    gl->glGenVertexArrays(1, &mVao);
  }
}

void CameraPath::add(glm::vec3 position, glm::vec3 lookAt)
{
  // Position is position, so just add it to the list without doing anything special.
  mPosVertices.emplace_back(position);

  // We are interested in interpolating the direction, but don't care about the
  // magnitude (because it's always 1). In order to avoid having the
  // interpolating function change the radius, we convert to spherical
  // coordinates and drop r.
  //
  // r = √x² + y² + z²
  // ϑ = tan⁻¹(y / x)
  // ϕ = cos⁻¹(z / r)

  //auto dir = glm::normalize(lookAt - position);
  //auto u = atan2(dir.y, dir.x); // ϑ
  //auto v = acos(dir.z); // ϕ

  //mDirVertices.emplace_back(u, v);
  mDirVertices.emplace_back(lookAt);
}

void CameraPath::buildSplines()
{
  mPosSplines.clear();
  mDirSplines.clear();
  auto& v = mPosVertices;
  auto& dir = mDirVertices;
  auto n = v.size();

  if (n < 3)
    return;

  for (int i = 0; i < n; i++) {
    auto a = (i > 0) ? i - 1 : n - 1;
    auto b = i;
    auto c = (i + 1) % n;
    auto d = (i + 2) % n;

    // Position spline
    {
      auto x = _hermite(v[a].x, v[b].x, v[c].x, v[d].x);
      auto y = _hermite(v[a].y, v[b].y, v[c].y, v[d].y);
      auto z = _hermite(v[a].z, v[b].z, v[c].z, v[d].z);
      glm::mat3x4 m { x, y, z };
      mPosSplines.emplace_back(glm::transpose(m));
    }

    // Direction spline
    {
      //auto u = _hermite(dir[a].x, dir[b].x, dir[c].x, dir[d].x);
      //auto v = _hermite(dir[a].y, dir[b].y, dir[c].y, dir[d].y);
      //glm::mat2x4 m { u, v };
      //println(":: {}, {}", dir[b].x, dir[b].y);
      //mDirSplines.emplace_back(glm::transpose(m));
      auto x = _hermite(dir[a].x, dir[b].x, dir[c].x, dir[d].x);
      auto y = _hermite(dir[a].y, dir[b].y, dir[c].y, dir[d].y);
      auto z = _hermite(dir[a].z, dir[b].z, dir[c].z, dir[d].z);
      glm::mat3x4 m { x, y, z };
      mDirSplines.emplace_back(glm::transpose(m));
    }
  }

  // Generate lines for drawing
  init();
  size_t num = n * SPLINE_STEPS;
  std::vector<glm::vec3> positions { num };
  //std::vector<glm::vec3> directions { 2 * num };
  std::vector<glm::vec3> directions { num };
  for (size_t i = 0; i < num; i++) {
    auto t = i / static_cast<float>(SPLINE_STEPS);
    auto pair = interp(t);

    positions[i] = pair.first;

    //directions[2 * i] = pair.first;
    //directions[2 * i + 1] = pair.first + pair.second * 0.1f;
    directions[i] = pair.second;
  }

  positions.insert(positions.end(), directions.cbegin(), directions.cend());

  gl->glBindVertexArray(mVao);
  gl->glBindBuffer(GL_ARRAY_BUFFER, mVbo);
  gl->glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * positions.size(), positions.data(), GL_STATIC_DRAW);
}

std::pair<glm::vec3, glm::vec3> CameraPath::interp(float t) const
{
  t = fmod(t, static_cast<float>(mPosSplines.size()));
  size_t i = t;
  t -= floor(t);
  Vec4 v { t*t*t, t*t, t, 1.0f };

  glm::vec3 pos = mPosSplines[i] * v;
  glm::vec3 dir = mDirSplines[i] * v;
  {
    // Gotta do a bit more work to convert the direction from spherical to
    // cartesian coordinates.
    //auto s = mDirSplines[i] * v;
    //dir.x = cos(s.x) * cos(s.y);
    //dir.y = cos(s.x) * sin(s.y);
    //dir.z = sin(s.x);
  }

  return std::make_pair(pos, dir);
}

void CameraPath::draw() const
{
  auto pathStart = 0;
  auto pathCount = mPosVertices.size() * SPLINE_STEPS;
  auto dirStart = pathStart + pathCount;
  auto dirCount = pathCount * 2;

  gl->glBindVertexArray(mVao);
  gl->glBindBuffer(GL_ARRAY_BUFFER, mVbo);
  gl->glEnableVertexAttribArray(0);
  gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, sizeof(GLfloat) * 3, 0);
  gl->glDrawArrays(GL_LINE_LOOP, pathStart, pathCount);
  gl->glDrawArrays(GL_LINES, dirStart, dirCount);
}
