#ifndef __INF251_LIGHTDIALOG__8685152
#define __INF251_LIGHTDIALOG__8685152

#include <QDialog>
#include "Renderer.hh"

namespace Ui {
  class LightDialog;
}

namespace View {
  class LightDialog : public QDialog {
    Q_OBJECT

  public:
    LightDialog(QWidget * parent);
    ~LightDialog();

    void show(Renderer::LightBlock &block, int light);

  protected:
    void closeEvent(QCloseEvent *evt) Q_DECL_OVERRIDE;

  private:
    void initialize();

    Ui::LightDialog * _ui;
    Renderer::LightBlock * _block;
    int _light = -1;
    bool _initialized = false;

    private slots:
    void on_chkEnable_toggled(bool value);
    void on_chkSpot_toggled(bool value);
    void on_sldRed_valueChanged(int value);
    void on_sldGreen_valueChanged(int value);
    void on_sldBlue_valueChanged(int value);
    void on_sldIntensity_valueChanged(int value);
    void on_sldAperture_valueChanged(int value);
  };
}

#endif //__INF251_LIGHTDIALOG__8685152