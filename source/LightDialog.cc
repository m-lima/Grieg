#include "LightDialog.hh"

#include "ui_LightDialog.h"
#include <QSignalMapper>

namespace View {
  LightDialog::LightDialog(QWidget * parent) :
    QDialog(parent),
    _ui(new Ui::LightDialog) {
    _ui->setupUi(this);
  }

  void LightDialog::show(Renderer::LightBlock &block, int light) {
    if (light == _light) {
      return;
    }

    _initialized = false;
    _block = &block;
    _light = light;

    if (_light == 0) {
      _ui->chkSpot->hide();
      _ui->lblAperture->hide();
      _ui->sldAperture->hide();
      setFixedSize(300, 270);
    } else {
      _ui->chkSpot->show();
      _ui->lblAperture->show();
      _ui->sldAperture->show();
      setFixedSize(300, 305);
    }
    
    initialize();
    QDialog::show();
  }
  
  void LightDialog::initialize() {
    _ui->chkEnable->setChecked(_block->type != 0);
    
    _ui->sldRed->setValue(_block->color.r * 255);
    _ui->sldGreen->setValue(_block->color.g * 255);
    _ui->sldBlue->setValue(_block->color.b * 255);
    
    _ui->sldIntensity->setValue(_block->intensity * 100);

    _ui->chkSpot->setChecked(_block->type == 3);
    _ui->sldAperture->setValue(_block->aperture * 200);

    _initialized = true;
  }

  void LightDialog::on_chkEnable_toggled(bool value) {
    if (!_initialized) {
      return;
    }

    if (value) {
      _block->type = _light == 0 ? 1 : (_ui->chkSpot->isChecked() ? 3 : 2);
    } else {
      _block->type = 0;
    }
  }

  void LightDialog::on_chkSpot_toggled(bool value) {
    if (!_initialized) {
      return;
    }

    if (_ui->chkEnable->isChecked()) {
      _block->type = value ? 3 : 2;
    }
  }

  void LightDialog::on_sldRed_valueChanged(int value) {
    if (!_initialized) {
      return;
    }

    _block->color.r = value / 100.0f;
  }

  void LightDialog::on_sldGreen_valueChanged(int value) {
    if (!_initialized) {
      return;
    }

    _block->color.g = value / 100.0f;
  }

  void LightDialog::on_sldBlue_valueChanged(int value) {
    if (!_initialized) {
      return;
    }

    _block->color.b = value / 100.0f;
  }

  void LightDialog::on_sldIntensity_valueChanged(int value) {
    if (!_initialized) {
      return;
    }

    _block->intensity = value / 100.0f;
  }

  void LightDialog::on_sldAperture_valueChanged(int value) {
    if (!_initialized) {
      return;
    }

    _block->aperture = value / 200.0f;
  }

}