#ifndef __INF251_BINPARSER__66745623
#define __INF251_BINPARSER__66745623

#include "infdef.hh"
#include <algorithm>
#include <glm/detail/func_geometric.hpp>

namespace {

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
    std::ifstream file(
      format("assets/meshes/{}.bin", name),
      std::ifstream::binary | std::ifstream::in
    );

    if (!file.is_open()) {
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
  std::vector<Vec3> generateVertices(const Terrain &terrain) {
    std::vector<Vec3> vertices;

    // Preemptive reservation of memory
    // Some values will be discarded and the vector might be shrunken
    // afterwards. This preemptive action will speed up the insertion
    vertices.reserve(terrain.header.columns * terrain.header.rows);

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
        vertices.push_back(Vec3(currentX, height * scale, currentZ));
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
    vertices.shrink_to_fit();

    return vertices;
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
  std::vector<Vec3> generateNormals(const Terrain &terrain, size_t size) {
    std::vector<Vec3> normals;

    // We are sure of the size; reserve the amount to push_back faster
    normals.reserve(size);

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
        normals.push_back(
          glm::normalize(Vec3(x, terrain.header.cellSize, z))
        );
      }
    }

    return normals;
  }

  /// Generates the texture coordinates
  ///
  /// This method assumes a regular grid is being parsed and, therefore,
  /// simply normalizes the X and Z grid coordinates into [0..1]
  std::vector<Vec2> generateTexCoords(const Terrain &terrain, size_t size) {
    std::vector<Vec2> coords;

    // We are sure of the size; reserve the amount to push_back faster
    coords.reserve(size);

    // Infer a X and Z coord system for easier coord calculation
    for (unsigned int col = 0; col < terrain.header.columns; ++col) {
      for (unsigned int row = 0; row < terrain.header.rows; ++row) {

        // Invalid vertex should be ignored
        if (terrain.getHeight(col, row) <= terrain.header.threshold) {
          continue;
        }

        coords.push_back(Vec2(
          static_cast<float>(col) / (terrain.header.columns - 1.0f),
          1.0f - (static_cast<float>(row) / (terrain.header.rows - 1.0f))
        ));
      }
    }

    return coords;
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
  std::vector<Vec3> generateFaces(Terrain &terrain, size_t size) {
    std::vector<Vec3> faces;

    // We don't know if we will need all this, but in worst-case
    // we will have two triangles per vertex
    // Actually: (col - 1)*(row - 1)*2, but close enough
    faces.reserve(size * 2);

    // Since some vertices are ignored, a mapping vector must be generated
    // to allow for proper reference pointing;
    std::vector<size_t> indexMapper;
    indexMapper.reserve(terrain.grid.size());
    int currentIndex = 1;
    for (auto & height : terrain.grid) {
      if (height > terrain.header.threshold) {
        indexMapper.push_back(currentIndex++);
      } else {
        indexMapper.push_back(0);

        // This will help see if there is an invalid vertex later
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
            faces.push_back(Vec3(
              indexMapper[col * terrain.header.rows + row],
              indexMapper[col * terrain.header.rows + row + 1],
              indexMapper[(col + 1) * terrain.header.rows + row + 1]
            ));
          }

          // Check if [C D A] is valid
          // Since we already checked A and C, only D needs to be checked
          if (d > 0) {
            faces.push_back(Vec3(
              indexMapper[(col + 1) * terrain.header.rows + row + 1],
              indexMapper[(col + 1) * terrain.header.rows + row],
              indexMapper[col * terrain.header.rows + row]
            ));
          }

        } else {
          // B-D is smaller and possibly valid
          // Using [A B D] [C D B]

          // Check if B-D is valid
          if (b * d > 0) {

            // Check if [A B D] is valid
            // Since we already checked B and D, only A needs to be checked
            if (a > 0) {
              faces.push_back(Vec3(
                indexMapper[col * terrain.header.rows + row],
                indexMapper[col * terrain.header.rows + row + 1],
                indexMapper[(col + 1) * terrain.header.rows + row]
              ));
            }

            // Check if [C D B] is valid
            // Since we already checked B and D, only C needs to be checked
            if (c > 0) {
              faces.push_back(Vec3(
                indexMapper[(col + 1) * terrain.header.rows + row + 1],
                indexMapper[(col + 1) * terrain.header.rows + row],
                indexMapper[col * terrain.header.rows + row + 1]
              ));
            }
          }
        }
      }
    }

    faces.shrink_to_fit();
    return faces;
  }
}

namespace BinParser {

  template <bool Normalize = true>
  void convertToObj(const std::string &name) {

    println("Converting {} from .bin to .obj", name);

    // Before we even start, check to see if we will be able to save at
    // end of the parsing process
    std::ofstream file(format("assets/meshes/{}.obj", name));

    if (!file.is_open()) {
      fatal("Cannot write into {}.obj", name);
    }
    fmt::print(file, "# Cached parsed file\n");
    fmt::print(file, "usemtl bergen_terrain_texture.png\n");

    size_t size;

    // Load the actual file into memory
    auto terrain = readTerrainFile(name);

    // Vertices
    {
      print("  Vertices generation:            ");

      // Convert into vertices
      auto vertices = generateVertices<Normalize>(terrain);

      // Capture the actual number of vertices
      size = vertices.size();

      // Save the vertices
      for (auto vertex : vertices) {
        fmt::print(file, "v {:f} {:f} {:f}\n", vertex.x, vertex.y, vertex.z);
      }

      // Free the used memory
      vertices = std::vector<Vec3>(0);

      println("DONE");
    }

    // Texture coordinates
    {
      print("  Texture coordinates generation: ");
      auto coords = generateTexCoords(terrain, size);
      for (auto coord : coords) {
        fmt::print(file, "vt {:f} {:f}\n", coord.x, coord.y);
      }
      coords = std::vector<Vec2>(0);
      println("DONE");
    }

    // Normals
    {
      print("  Normals generation:             ");
      auto normals = generateNormals(terrain, size);
      for (auto normal : normals) {
        fmt::print(file, "vn {:f} {:f} {:f}\n", normal.x, normal.y, normal.z);
      }
      normals = std::vector<Vec3>(0);
      println("DONE");
    }

    // Faces
    {
      print("  Faces generation:               ");
      auto faces = generateFaces(terrain, size);
      for (auto face : faces) {
        fmt::print(
          file,
          "f {0:.0f}/{0:.0f}/{0:.0f}"
          " {1:.0f}/{1:.0f}/{1:.0f}"
          " {2:.0f}/{2:.0f}/{2:.0f}\n",
          face.x,
          face.y,
          face.z
        );
      }
      faces = std::vector<Vec3>(0);
      println("DONE");
    }
  }
}

#endif //__INF251_BINPARSER__66745623
