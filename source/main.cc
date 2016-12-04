#include <fstream>
#include <sstream>

#include "Renderer.hh"

#include <QApplication>
#include <QDesktopWidget>
#include <QTextStream>
#include <QSplashScreen>

#include "MainWindow.hh"

#ifdef _WIN32
// Force high performance GPU
extern "C" {
  // NVidia
  __declspec(dllexport) DWORD NvOptimusEnablement = 1;

  // AMD
  __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

QOpenGLFunctions_4_3_Core *gl = nullptr;

std::string readFileContents(const std::string &file) {
  std::ifstream fh(file, std::ios::binary);
  if (!fh.is_open())
    throw std::runtime_error(format("Couldn't open file {}", file));

  std::string buf;

  fh.seekg(0, std::ios::end);
  auto length = fh.tellg();
  buf.reserve(static_cast<size_t>(length));
  fh.seekg(0, std::ios::beg);
  buf.assign(std::istreambuf_iterator<char>(fh), std::istreambuf_iterator<char>());

  return buf;
}

void center(QWidget & widget) {
  int x, y;
  int screenWidth;
  int screenHeight;

  int WIDTH = widget.width();
  int HEIGHT = widget.height();

  QDesktopWidget *desktop = QApplication::desktop();

  screenWidth = desktop->screen()->width();
  screenHeight = desktop->screen()->height();

  x = (screenWidth - WIDTH) / 2;
  y = (screenHeight - HEIGHT) / 2;

  widget.move(x, y);
}

#if defined(_WIN32) && defined(NDEBUG)
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char*, int nShowCmd) {
  int argc = 0;
  QApplication app(argc, 0);
#else
int main(int argc, char * argv[]) {
  QApplication app(argc, argv);
#endif

  {
    QFile style(":qdarkstyle/style.qss");
    style.open(QFile::ReadOnly | QFile::Text);
    QTextStream stream(&style);
    app.setStyleSheet(stream.readAll());
  }

  app.setApplicationName("Grieghallen Explorer");

  QSurfaceFormat surfaceFormat;
  surfaceFormat.setDepthBufferSize(24);
  surfaceFormat.setStencilBufferSize(8);
  surfaceFormat.setVersion(4, 3);
  surfaceFormat.setProfile(QSurfaceFormat::CoreProfile);
  QSurfaceFormat::setDefaultFormat(surfaceFormat);

  View::MainWindow mainWindow;
  mainWindow.setWindowIcon(QIcon(":images/icon2.png"));
  
  QSplashScreen splash(&mainWindow);
  splash.setPixmap(QPixmap(":images/splash.png"));
  splash.show();

  Renderer renderer(&mainWindow);
  renderer.setFormat(surfaceFormat);
  gl = &renderer;

  mainWindow.resize(1024, 768);
  center(mainWindow);
  mainWindow.attachRenderer(&renderer);

#ifdef NDEBUG
  mainWindow.showFullScreen();
#else
  mainWindow.show();
#endif
  
  splash.finish(&mainWindow);

  return app.exec();
}
