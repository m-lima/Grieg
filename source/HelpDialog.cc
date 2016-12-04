#include "HelpDialog.hh"

namespace View {
  HelpDialog::HelpDialog(QWidget *parent) :
    QDialog(parent) {
    setupUi();
    setModal(true);
    
    adjustSize();
    setFixedSize(300, height());
  }

  void HelpDialog::setupUi() {
    lytMain = new QVBoxLayout(this);
    lytTexts = new QHBoxLayout();
    lblKeys = new QLabel(this);
    lblActions = new QLabel(this);

    buttonBox = new QDialogButtonBox(this);
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok);

    lytTexts->addWidget(lblKeys);
    lytTexts->addWidget(lblActions);
    lytMain->addLayout(lytTexts);
    lytMain->addWidget(buttonBox);

    retranslateUi();

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  }

  void HelpDialog::retranslateUi() {
    setWindowTitle("Help");
    lblKeys->setText(
      "<html><head/><body>"
      "<b>Global:</b><br>"
      "Wheel mouse<br>"
      "Space<br>"
      "F<br>"
      "X<br>"
      "Escape<br>"
      "?<br>"
      "<br>"
      "<b>Trackball mode:</b><br>"
      "Left mouse<br>"
      "Left mouse + CTRL<br>"
      "Right mouse<br>"
      "<br>"
      "<b>WASD mode:</b><br>"
      "W<br>"
      "A<br>"
      "S<br>"
      "D<br>"
      "Shift<br>"
      "CTRL<br>"
      "<br>"
      "<b>Path mode:</b><br>"
      "Left mouse + CTRL<br>"
      "</body></html>"
      );
    lblActions->setText(
      "<html><head/><body>"
      "<br>"
      "Zoom<br>"
      "Toggle projection<br>"
      "Toggle fullscreen<br>"
      "Reset view<br>"
      "Quit<br>"
      "Show this help<br>"
      "<br>"
      "<br>"
      "Rotate<br>"
      "Rotate Sun<br>"
      "Translate<br>"
      "<br>"
      "<br>"
      "Move forward<br>"
      "Move left<br>"
      "Move backward<br>"
      "Move right<br>"
      "Move up<br>"
      "Move down<br>"
      "<br>"
      "<br>"
      "Rotate Sun<br>"
      "</body></html>"
      );
  }
}
