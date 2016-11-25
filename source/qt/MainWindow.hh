#ifndef __INF251_QT_MAINWINDOW__2733556
#define __INF251_QT_MAINWINDOW__2733556

#include "../infdef.hh"

#include <QMainWindow>

#include "OpenGLWidget.hh"

namespace {

#ifdef GLAD_DEBUG
  void glad_pre_callback(const char *, void *, int, ...) {}

  void glad_post_callback(const char *name, void *, int, ...) {
    GLenum err;
    err = glad_glGetError();
    if (err == GL_NO_ERROR)
      return;
    println(stderr, "GL error(s) occurred after calling {} with error #{}:", name, err);

    int numLogs;
    glGetIntegerv(GL_DEBUG_LOGGED_MESSAGES, &numLogs);

    assert(numLogs > 0);

    char log[1024];
#if defined(_MSC_VER)
    auto sources = std::make_unique<GLenum[]>(numLogs);
    auto types = std::make_unique<GLenum[]>(numLogs);
    auto ids = std::make_unique<GLuint[]>(numLogs);
    auto severities = std::make_unique<GLenum[]>(numLogs);
    auto lengths = std::make_unique<GLsizei[]>(numLogs);
    glad_glGetDebugMessageLog(numLogs, sizeof(log), sources.get(), types.get(), ids.get(), severities.get(), lengths.get(), log);
#else
    GLenum sources[numLogs];
    GLenum types[numLogs];
    GLuint ids[numLogs];
    GLenum severities[numLogs];
    GLsizei lengths[numLogs];
    glad_glGetDebugMessageLog(numLogs, sizeof(log), sources, types, ids, severities, lengths, log);
#endif

    size_t off = 0;
    println("--- GL MESSAGE LOG BEGIN ---");
    for (int i = 0; i < numLogs; i++) {
      println(stderr, "{}", log + off);
      off += lengths[i];
    }
    //for (auto&& len : lengths) {
    //    println(stderr, "{}", log + off);
    //    off += len;
    //}
    println("---- GL MESSAGE LOG END ----");

    std::terminate();
  }
#endif //GLAD_DEBUG
}

namespace uiQT {
  class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    MainWindow();
    ~MainWindow() = default;

    void attachRenderer(OpenGLWidget * openGL);
  };
}

#endif //__INF251_QT_MAINWINDOW__2733556
