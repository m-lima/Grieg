#include <map>
#include "Object.hh"
#include <glm/gtc/matrix_transform.hpp>
#include <thread>
#include <algorithm>

#include <QFile>
#include <QTextStream>

#ifdef _WIN32
#include <io.h>
#define access _access_s
#else
#include <unistd.h>
#endif // _WIN32


namespace {
  // Returns true if token was read successfully
  template<char Char = ' ', bool Skip = true>
  bool nextTokenPos(const std::string &line, size_t &left, size_t &right) {
    if (right == std::string::npos)
      return false;

    left = Skip ? line.find_first_not_of(Char, right) : (right == 0 ? right : right + 1);
    right = line.find_first_of(Char, left);

    return true;
  }

  template<char Char = ' ', bool Skip = true>
  bool nextToken(const std::string &line, std::string &out, size_t &left, size_t &right) {
    if (nextTokenPos<Char, Skip>(line, left, right)) {
      out = line.substr(left, right - left);
      return !out.empty();
    }
    return false;
  }

  template<char Char = ' ', bool Skip = true>
  bool nextToken(const std::string &line, float &out, size_t &left, size_t &right) {
    std::string tok;
    if (nextToken<Char, Skip>(line, tok, left, right)) {
      out = std::stod(tok);
      return true;
    }
    return false;
  }

  template<char Char = ' ', bool Skip = true>
  bool nextToken(const std::string &line, int &out, size_t &left, size_t &right) {
    std::string tok;
    if (nextToken<Char, Skip>(line, tok, left, right)) {
      out = std::stoi(tok);
      return true;
    }
    return false;
  }

  struct Vertex {
    glm::vec3 pos;
    glm::vec2 tex;
    glm::vec3 norm;

    constexpr Vertex(const glm::vec3 &pPos,
                     const glm::vec2 &pTex,
                     const glm::vec3 &pNorm) :
      pos(pPos),
      tex(pTex),
      norm(pNorm) {}
  };
  static_assert(sizeof(Vertex) == sizeof(GLfloat) * 8, "sizeof Vertex is incorrect");

  struct ObjFile {
    struct Material {
      std::string name = "";
      size_t count = 0;
    };

    struct Triangle {
      size_t matIdx;
      glm::ivec3 posIdx;
      glm::ivec3 texIdx;
      glm::ivec3 normIdx;
    };

    std::string materialLib{};
    std::vector<Material> materials{ 1 };
    std::vector<glm::vec3> positions{};
    std::vector<glm::vec3> normals{};
    std::vector<glm::vec2> texCoords{};
    std::vector<Triangle> trigs{};
  };

  ObjFile readObjFile(const std::string &name) {
    ObjFile obj;

    auto path = format("resources/meshes/{}", name);

    QFile file(QString::fromStdString(path));
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
      fatal("Couldn't open file {}.obj", name);
    }
    QTextStream stream(&file);

    println("Loading mesh: {}", name);

    while (!stream.atEnd()) {
      std::string line, tmp;
      line = stream.readLine().toStdString();

      if (line.empty() || line[0] == '#')
        continue;

      size_t left, right = 0;
      nextToken(line, tmp, left, right);

      /*
       * <x:f>: x is a required float
       * [w:i]: w is an optional integer
       */

      if (tmp == "mtllib") {
        nextToken(line, obj.materialLib, left, right);
      } else if (tmp == "usemtl") {
        ObjFile::Material mat;
        nextToken(line, mat.name, left, right);
        obj.materials.push_back(mat);
      } else if (tmp == "v") {
        /* Vertex: v <x:f> <y:f> <z:f> [w:f] */
        glm::vec3 vec;
        nextToken(line, vec.x, left, right);
        nextToken(line, vec.y, left, right);
        nextToken(line, vec.z, left, right);

        float w;
        if (nextToken(line, w, left, right)) {
          vec /= w;
        }

        obj.positions.emplace_back(vec);
      } else if (tmp == "vt") {
        /* Vertex texture coordinate: vt <s:f> <t:f> */
        glm::vec2 vec;
        nextToken(line, vec.s, left, right);
        nextToken(line, vec.t, left, right);
        vec.t = 1.0f - vec.t;
        obj.texCoords.emplace_back(vec);
      } else if (tmp == "vn") {
        /* Vertex normal: vn <x:f> <y:f> <z:f> */
        glm::vec3 vec;
        nextToken(line, vec.x, left, right);
        nextToken(line, vec.y, left, right);
        nextToken(line, vec.z, left, right);
        obj.normals.emplace_back(glm::normalize(vec));
      } else if (tmp == "f") {
        /* Face: Kind of complicated, read the wiki */
        std::string subtok;
        size_t subleft, subright;
        glm::ivec3 v, vt, vn;
        bool tex = false;
        bool norm = false;

        /* Read first space-separated token, x */
        subleft = subright = 0;
        nextToken(line, subtok, left, right);
        nextToken<'/', false>(subtok, v.x, subleft, subright);
        if (nextToken<'/', false>(subtok, vt.x, subleft, subright)) tex = true;
        if (nextToken<'/', false>(subtok, vn.x, subleft, subright)) norm = true;

        /* Read y */
        subleft = subright = 0;
        nextToken(line, subtok, left, right);
        nextToken<'/', false>(subtok, v.y, subleft, subright);
        nextToken<'/', false>(subtok, vt.y, subleft, subright);
        nextToken<'/', false>(subtok, vn.y, subleft, subright);

        /* Read z */
        subleft = subright = 0;
        nextToken(line, subtok, left, right);
        nextToken<'/', false>(subtok, v.z, subleft, subright);
        nextToken<'/', false>(subtok, vt.z, subleft, subright);
        nextToken<'/', false>(subtok, vn.z, subleft, subright);

        obj.trigs.push_back({ obj.materials.size() - 1, v, vt, vn });
        obj.materials.back().count++;
      }
    }

    file.close();

    return obj;
  }

  struct MtlFile {
    struct Material {
      glm::vec3 ambient{};
      glm::vec3 diffuse{};
      glm::vec3 specular{};
      std::shared_ptr<Texture> texture{};
    };

    std::map<std::string, Material> materials;
  };

  MtlFile readMtlFile(const std::string &name) {
    MtlFile mtl;

    auto path = format("resources/meshes/{}", name);

    QFile file(QString::fromStdString(path));
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
      fatal("Couldn't open material file {}", name);
    }
    QTextStream stream(&file);

    println("Loading materials: {}", name);

    MtlFile::Material *mat = nullptr;

    while (!stream.atEnd()) {
      std::string line, tmp;
      line = stream.readLine().toStdString();

      if (line.empty() || line[0] == '#')
        continue;

      size_t left, right = 0;
      nextToken(line, tmp, left, right);

      if (tmp == "newmtl") {
        std::string name;
        nextToken(line, name, left, right);
        mat = &mtl.materials[name];
      } else if (tmp == "Ka") {
        auto& vec = mat->ambient;
        nextToken(line, vec.r, left, right);
        nextToken(line, vec.g, left, right);
        nextToken(line, vec.b, left, right);
      } else if (tmp == "Kd") {
        auto& vec = mat->diffuse;
        nextToken(line, vec.r, left, right);
        nextToken(line, vec.g, left, right);
        nextToken(line, vec.b, left, right);
      } else if (tmp == "Ks") {
        auto& vec = mat->specular;
        nextToken(line, vec.r, left, right);
        nextToken(line, vec.g, left, right);
        nextToken(line, vec.b, left, right);
      } else if (tmp == "map_Kd") {
        std::string str;
        nextToken(line, str, left, right);
        mat->texture = std::make_shared<Texture>();
        mat->texture->load(str);
      }
    }

    return mtl;
  }


  // A packed struct of the header for direct reading
#pragma pack(push, 1)
  struct TerrainHeader {
    unsigned int columns;
    unsigned int rows;
    double xStart;
    double zStart;
    double cellSize;
    int threshold;
  };
#pragma pack(pop)

  // Full data holder
  struct Terrain {
    TerrainHeader header;
    std::vector<float> grid;

    Terrain(const TerrainHeader &newHeader) :
      header(std::move(newHeader)),
      grid(newHeader.columns * newHeader.rows) {}

    /// Convenience method for accessing height values
    float getHeight(int x, int z) const {

      // Horizontal bound check
      if (x < 0 || x >= header.columns) {
        return static_cast<float>(header.threshold);
      }

      // Axial bound check
      if (z < 0 || z >= header.rows) {
        return static_cast<float>(header.threshold);
      }

      // The grid might have been cleared
      int index = x * header.rows + z;
      if (index >= grid.size()) {
        return static_cast<float>(header.threshold);
      }

      // Fast access with no bound check, since it was already performed
      return grid[index];
    }
  };

  /// Reads a .bin file and loads all its contents into memory
  Terrain readTerrainFile(const std::string & name) {
    auto path = format("resources/meshes/{}", name);
    QFile file(QString::fromStdString(path));
    if (!file.open(QFile::ReadOnly)) {
      fatal("Couldn't open file {}.bin", name);
    }

    TerrainHeader header;

    // Read the header
    file.read(reinterpret_cast<char*>(&header), sizeof(TerrainHeader));
    //header.threshold = 0;
    println("  columns:        {}", header.columns);
    println("  rows:           {}", header.rows);
    println("  X start:        {}", header.xStart);
    println("  Z start:        {}", header.zStart);
    println("  cell size:      {}", header.cellSize);
    println("  threshold:      {}", header.threshold);
    Terrain terrain(header);

    // Read the grid points
    file.read(
      reinterpret_cast<char*>(terrain.grid.data()),
      terrain.grid.size() * sizeof(float)
    );

    file.close();

    return terrain;
  }

  /// Generates OpenGL-compatible vertices for the .bin file
  /// The .obj can be normalized between [-1..1], if requested
  template <bool Normalize>
  void generateVertices(const Terrain &terrain, std::vector<Vec3> * vertices) {

    // Prepare global coordinates (use memory in lieu of recomputing
    // the multiplication for every vertex)
    double currentX = Normalize ? -1.0 : terrain.header.xStart;
    double currentZ = Normalize ? -1.0 : terrain.header.zStart;
    unsigned int row = 0;
    unsigned int col = 0;

    // If we are normalizing, prepare the factor
    float scale =
      Normalize ? 2.0f / std::abs(terrain.header.cellSize *
      (std::max(terrain.header.columns, terrain.header.rows) - 1.0f))
      : 1.0f;

    for (auto height : terrain.grid) {

      // Some values are invalid. Only add if if higher than the threshold
      if (height > terrain.header.threshold) {
        vertices->emplace_back(currentX, height * scale, currentZ);
      }

      // Move to the next position
      row++;
      currentZ += terrain.header.cellSize * scale;

      // And overflow if necessary
      if (row >= terrain.header.rows) {
        row = 0;
        currentZ = Normalize ? -1.0 : terrain.header.zStart;
        col++;
        currentX += terrain.header.cellSize * scale;
      }
    }

    // Shrink the vector. We might not need it to be so big
    vertices->shrink_to_fit();

    println("Vertex loading done");
  }

  /// Generates the texture coordinates
  ///
  /// This method assumes a regular grid is being parsed and, therefore,
  /// simply normalizes the X and Z grid coordinates into [0..1]
  void generateTexCoords(const Terrain &terrain, std::vector<Vec2> * coords) {

    // Infer a X and Z coord system for easier coord calculation
    for (unsigned int col = 0; col < terrain.header.columns; ++col) {
      for (unsigned int row = 0; row < terrain.header.rows; ++row) {

        // Invalid vertex should be ignored
        if (terrain.getHeight(col, row) <= terrain.header.threshold) {
          continue;
        }

        coords->emplace_back(
          static_cast<float>(col) / (terrain.header.columns - 1.0f),
          (static_cast<float>(row) / (terrain.header.rows - 1.0f))
        );
      }
    }

    // Shrink the vector. We might not need it to be so big
    coords->shrink_to_fit();

    println("Texture coordinate loading done");
  }

  /// Generates the normals for each vertex
  ///
  /// It works by calculating the derivative of the imediate neighbors'
  /// heights and placing them in a `Vec3`. Y will be set to `Header.CellSize`
  /// and the vector will be normalized.
  ///
  /// That means that, if there is no gradient in the X or Z axes, the normal
  /// will simply face up with a unit value.
  ///
  /// If a given vertex does not contain a particular neighbor, its own 
  /// height will be used for derivation
  void generateNormals(const Terrain &terrain, std::vector<Vec3> * normals) {

    // Reserve variables for the gradient
    float h, n, s, e, w, x, z;

    // Infer a X and Z coord system for easier gradient calculation
    for (unsigned int col = 0; col < terrain.header.columns; ++col) {
      for (unsigned int row = 0; row < terrain.header.rows; ++row) {
        h = terrain.getHeight(col, row);

        // Invalid vertex should be ignored
        if (h <= terrain.header.threshold) {
          continue;
        }

        n = terrain.getHeight(col, row + 1);
        s = terrain.getHeight(col, row - 1);
        e = terrain.getHeight(col + 1, row);
        w = terrain.getHeight(col - 1, row);

        // We also cannot use invalid vertices for gradient calculation
        if (n <= terrain.header.threshold) {
          n = h;
        }
        if (s <= terrain.header.threshold) {
          s = h;
        }
        if (e <= terrain.header.threshold) {
          e = h;
        }
        if (w <= terrain.header.threshold) {
          w = h;
        }

        // Calculate gradient
        x = s - n;
        z = w - e;

        // Push back the normalize normal vector
        normals->push_back(
          glm::normalize(Vec3(x, terrain.header.cellSize, z))
        );
      }
    }

    // Shrink the vector. We might not need it to be so big
    normals->shrink_to_fit();

    println("Normal loading done");
  }

  /// Generates the indices for drawing the faces
  ///
  /// Since vertex, normal, and texture coordinates will all be the same,
  /// the method returns a simple `Vec3` that represents the indices
  ///
  /// There are only two options given a quad represented by A B C D in
  /// clockwise order and with A at the lower left corner: 
  /// [A B C] [C D A]
  /// or
  /// [A B D] [C D B]
  ///
  /// The defining value will then be the smaller of A.y - C.y and B.y - D.y
  void generateFaces(Terrain &terrain, size_t size, std::vector<glm::ivec3> * faces) {

    // We don't know if we will need all this, but in worst-case
    // we will have two triangles per vertex
    // Actually: (col - 1)*(row - 1)*2, but close enough
    faces->reserve(size * 2);

    // Since some vertices are ignored, a mapping vector must be generated
    // to allow for proper reference pointing;
    std::vector<size_t> indexMapper;
    indexMapper.reserve(terrain.grid.size());
    int currentIndex = 0;
    for (auto & height : terrain.grid) {
      if (height > terrain.header.threshold) {
        indexMapper.push_back(currentIndex++);
      } else {
        indexMapper.push_back(0);

        // This will help to see if there is an invalid vertex later
        height = static_cast<float>(terrain.header.threshold);
      }
    }

    // Reserve variables for the evaluated quad
    // B - C
    // |   |
    // A - D
    float a, b, c, d;

    // Infer a X and Z coord system for easier index calculation
    for (unsigned int col = 0; col < terrain.header.columns; ++col) {
      for (unsigned int row = 0; row < terrain.header.rows; ++row) {

        // We substract the thresholh because above we know that no number
        // is less than it. This will facilitate the check later
        a = terrain.getHeight(col, row) - terrain.header.threshold;
        b = terrain.getHeight(col, row + 1) - terrain.header.threshold;
        c = terrain.getHeight(col + 1, row + 1) - terrain.header.threshold;
        d = terrain.getHeight(col + 1, row) - terrain.header.threshold;

        // First, check the smaller delta Y
        // Then verify that the diagonal is valid (larger than 0)
        if (std::abs(a - c) < std::abs(b - d) && a * c > 0) {
          // A-C is smaller and is valid
          // Using [A B C] [C D A]

          // Check if [A B C] is valid
          // Since we already checked A and C, only B needs to be tested
          if (b > 0) {
            faces->emplace_back(
              indexMapper[col * terrain.header.rows + row],
              indexMapper[col * terrain.header.rows + row + 1],
              indexMapper[(col + 1) * terrain.header.rows + row + 1]
            );
          }

          // Check if [C D A] is valid
          // Since we already checked A and C, only D needs to be checked
          if (d > 0) {
            faces->emplace_back(
              indexMapper[(col + 1) * terrain.header.rows + row + 1],
              indexMapper[(col + 1) * terrain.header.rows + row],
              indexMapper[col * terrain.header.rows + row]
            );
          }

        } else {
          // B-D is smaller and possibly valid
          // Using [A B D] [C D B]

          // Check if B-D is valid
          if (b * d > 0) {

            // Check if [A B D] is valid
            // Since we already checked B and D, only A needs to be checked
            if (a > 0) {
              faces->emplace_back(
                indexMapper[col * terrain.header.rows + row],
                indexMapper[col * terrain.header.rows + row + 1],
                indexMapper[(col + 1) * terrain.header.rows + row]
              );
            }

            // Check if [C D B] is valid
            // Since we already checked B and D, only C needs to be checked
            if (c > 0) {
              faces->emplace_back(
                indexMapper[(col + 1) * terrain.header.rows + row + 1],
                indexMapper[(col + 1) * terrain.header.rows + row],
                indexMapper[col * terrain.header.rows + row + 1]
              );
            }
          }
        }
      }
    }
  }
}

Object::~Object() {
  if (!mVbo)
    gl->glDeleteBuffers(1, &mVbo);

  if (!mIbo)
    gl->glDeleteBuffers(1, &mIbo);

  if (mVao)
    gl->glDeleteVertexArrays(1, &mVao);
}

void Object::init() {
  if (!mVbo)
    gl->glGenBuffers(1, &mVbo);

  if (!mIbo)
    gl->glGenBuffers(1, &mIbo);

  if (!mVao)
    gl->glGenVertexArrays(1, &mVao);
}

void Object::load(const std::string &name) {
  if (name.length() < 5) {
    fatal("Invalid mesh: {}", name);
  }

  if (std::equal(name.end() - 4, name.end(), ".obj")) {
    loadObjFile(name);
  } else if (std::equal(name.end() - 4, name.end(), ".bin")) {
    loadBinFile(name);
  } else {
    fatal("Unrecognized file name: {}", name);
  }
}

void Object::loadObjFile(const std::string &name) {
  auto obj = readObjFile(name);
  auto mtl = !obj.materialLib.empty() ? readMtlFile(obj.materialLib) : MtlFile{};

  /* The indices start from 1 and 0, so subtract if positive.
  * If negative, then the index refers to the vertices from the end of the list
  * ie. -1 is the last vertex, -2 is next to last, and so on. */
  const auto posSize = static_cast<int>(obj.positions.size());
  const int texSize = static_cast<int>(obj.texCoords.size());
  const int normSize = static_cast<int>(obj.normals.size());
  const auto normalizeIdx = [](int size, glm::ivec3 &v) {
    v.x = (v.x > 0) ? v.x - 1 : size + v.x - 1;
    v.y = (v.y > 0) ? v.y - 1 : size + v.y - 1;
    v.z = (v.z > 0) ? v.z - 1 : size + v.z - 1;
  };
  std::vector<Vertex> vertices;
  std::vector<GLuint> indices;

  MaterialGroup *mat = nullptr;
  size_t matIdx = 0;
  for (auto &f : obj.trigs) {
    normalizeIdx(posSize, f.posIdx);
    normalizeIdx(texSize, f.texIdx);
    normalizeIdx(normSize, f.normIdx);

    indices.emplace_back(vertices.size());
    vertices.emplace_back(obj.positions[f.posIdx.x],
                          obj.texCoords[f.texIdx.x],
                          obj.normals[f.normIdx.x]);

    indices.emplace_back(vertices.size());
    vertices.emplace_back(obj.positions[f.posIdx.y],
                          obj.texCoords[f.texIdx.y],
                          obj.normals[f.normIdx.y]);

    indices.emplace_back(vertices.size());
    vertices.emplace_back(obj.positions[f.posIdx.z],
                          obj.texCoords[f.texIdx.z],
                          obj.normals[f.normIdx.z]);

    if (!mat || f.matIdx != matIdx) {
      auto&& objMat = obj.materials[matIdx];
      auto&& mtlMat = mtl.materials[objMat.name];
      if (obj.materialLib.empty()) {
        mMaterialGroups.push_back({ objMat.count, nullptr, nullptr, mtlMat.ambient, mtlMat.diffuse, mtlMat.specular });
      } else {
        mMaterialGroups.push_back({ objMat.count, mtlMat.texture, nullptr, mtlMat.ambient, mtlMat.diffuse, mtlMat.specular });
      }
      matIdx = f.matIdx;
      mat = &mMaterialGroups.back();
    }
  }

  println("  positions:      {}", obj.positions.size());
  println("  texcoords:      {}", obj.texCoords.size());
  println("  normals:        {}", obj.normals.size());
  println("  faces:          {}", obj.trigs.size());
  println("  materials:      {}", mMaterialGroups.size());
  println("  vertices:       {}", vertices.size());
  println("  indices:        {}", indices.size());

  init();

  gl->glBindVertexArray(mVao);

  gl->glBindBuffer(GL_ARRAY_BUFFER, mVbo);
  gl->glBufferData(GL_ARRAY_BUFFER,
                   vertices.size() * sizeof(vertices[0]),
                   &vertices[0],
                   GL_STATIC_DRAW);

  gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIbo);
  gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                   indices.size() * sizeof(indices[0]),
                   &indices[0],
                   GL_STATIC_DRAW);

  mTrigCount = static_cast<GLuint>(indices.size());

}

template <bool Normalize>
void Object::loadBinFile(const std::string &name) {

  println("Loading {} as a bin file", name);

  // Load the actual file into memory
  auto terrain = readTerrainFile(name);

  // Preemptive reservation of memory
  // Some values will be discarded and the vector might be shrunken
  // afterwards. This preemptive action will speed up the insertion
  size_t size = terrain.header.columns * terrain.header.rows;

  // Launch one thread for each sub-step
  std::vector<std::thread> threads;
  threads.reserve(3);

  // Vertices
  std::vector<Vec3> vertices;
  vertices.reserve(size);
  threads.emplace_back(&generateVertices<Normalize>, terrain, &vertices);

  // Texture coordinates
  std::vector<Vec2> coords;
  coords.reserve(size);
  threads.emplace_back(&generateTexCoords, terrain, &coords);

  // Normals
  std::vector<Vec3> normals;
  normals.reserve(size);
  threads.emplace_back(&generateNormals, terrain, &normals);

  for (int i = 0; i < threads.size(); ++i) {
    threads[i].join();
  }

  // Capture the actual number of vertices
  size = vertices.size();

  println("All threads done. {} points loaded", size);

  // Fork the indices creation. This can be done in parallel with loading the
  // GL buffers
  std::vector<glm::ivec3> faces;
  std::thread faceThread(&generateFaces, std::ref(terrain), size, &faces);

  // Load the vertex, normal, and texture coordinate buffers
  std::vector<Vertex> buffer;
  buffer.reserve(size);
  for (size_t i = 0; i < size; ++i) {
    buffer.emplace_back(vertices[i], coords[i], normals[i]);
  }

  // Free unused memory
  vertices = std::vector<Vec3>(0);
  coords = std::vector<Vec2>(0);
  normals = std::vector<Vec3>(0);

  // Send it to OpenGL
  init();
  gl->glBindBuffer(GL_ARRAY_BUFFER, mVbo);
  gl->glBufferData(GL_ARRAY_BUFFER,
                   buffer.size() * sizeof(buffer[0]),
                   &buffer[0],
                   GL_STATIC_DRAW);

  // Wait for the face generation to finish
  faceThread.join();

  gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIbo);
  gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                   faces.size() * sizeof(faces[0]),
                   &faces[0],
                   GL_STATIC_DRAW);

  mTrigCount = static_cast<GLuint>(faces.size());
}

void Object::update() {
  glm::mat4 mat{};
  mat = glm::translate(mat, mPosition);
  mat = mat * modelTransform;

  mShader->uniform("uModel") = mat;
}

void Object::bind() {
  gl->glBindVertexArray(mVao);

  gl->glBindBuffer(GL_ARRAY_BUFFER, mVbo);
  gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIbo);

  gl->glEnableVertexAttribArray(0);
  gl->glVertexAttribPointer(0,
                            3,
                            GL_FLOAT,
                            GL_TRUE,
                            sizeof(Vertex),
                            reinterpret_cast<const void*>(offsetof(Vertex, pos)));

  gl->glEnableVertexAttribArray(1);
  gl->glVertexAttribPointer(1,
                            2,
                            GL_FLOAT,
                            GL_TRUE,
                            sizeof(Vertex),
                            reinterpret_cast<const void*>(offsetof(Vertex, tex)));

  gl->glEnableVertexAttribArray(2);
  gl->glVertexAttribPointer(2,
                            3,
                            GL_FLOAT,
                            GL_TRUE,
                            sizeof(Vertex),
                            reinterpret_cast<const void*>(offsetof(Vertex, norm)));
}

void Object::draw() {
  update();
  mShader->use();
  mShader->bindBuffer(matBlock);
  bind();
  const GLuint *start = nullptr;
  for (const auto &mat : mMaterialGroups) {
    if (mat.texture && enableTexture) {
      mat.texture->bind();
      mShader->uniform("uHaveTexture") = 1;
      matBlock->ambient = mat.ambient;
      matBlock->diffuse = mat.diffuse;
      matBlock->specular = mat.specular;
    } else {
      matBlock->ambient = glm::vec3(0.0f);
      matBlock->diffuse = glm::vec3(0.5f);
      matBlock->specular = glm::vec3(0.3f);
      mShader->uniform("uHaveTexture") = 0;
    }

    matBlock.update();

    auto count = mat.count;

    if (mMaterialGroups.size() == 1)
      count = mTrigCount;

    if (mat.bump) {
      mat.bump->bind(2);
      mShader->uniform("uHaveBump") = 1;
    } else {
      mShader->uniform("uHaveBump") = 0;
    }

    gl->glDrawElements(GL_TRIANGLES, count * 3, GL_UNSIGNED_INT, start);
    start += count * 3;
  }
  // mShader->unbindBuffer(matBlock);
}
