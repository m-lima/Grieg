#ifndef __INF251_INFDEF__54090249
#define __INF251_INFDEF__54090249

/**
 * This header defines the defaults for the INF251 project and is
 * meant to be included in every translation unit.
 */

//#if __cplusplus < 201103L
//#error "A C++11 compiler is needed to compile this project"
//#endif

#include <cstdint>
#include <fmt/format.h>
#include <fmt/ostream.h>

//#include <glad/glad.h>
#include <QOpenGLFunctions_4_3_Core>
#if defined(__linux__) || defined(_WIN32)
#include <GL/gl.h>
#else
#include <OpenGL/gl.h>
#endif

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>

using Vec2 = glm::vec2;
using Vec3 = glm::vec3;
using Vec4 = glm::vec4;
using Mat3 = glm::mat3;
using Mat4 = glm::mat4;
using Quat = glm::quat;

extern QOpenGLFunctions_4_3_Core *gl;

namespace fmt {
    /* fmtlib doesn't have a println function, so we add our own */
    inline void println(CStringRef format, ArgList args) {
        print(format, args);
        print("\n");
    }

    inline void println(std::FILE *fh, CStringRef format, ArgList args) {
        print(fh, format, args);
        print(fh, "\n");
    }

    /* write something and promptly die */
    inline void fatal(CStringRef format, ArgList args) {
        println(stderr, format, args);
        std::terminate();
    }

    FMT_VARIADIC(void, println, CStringRef)
    FMT_VARIADIC(void, println, std::FILE*, CStringRef)
    FMT_VARIADIC(void, fatal, CStringRef)
}

using namespace fmt;

std::string readFileContents(const std::string &path);

#endif //__INF251_INFDEF__54090249
