#include "HelpDialog.hh"

namespace View {
  HelpDialog::HelpDialog(QWidget *parent) :
    QDialog(parent) {
    setupUi();
    setModal(true);
  }

  void HelpDialog::setupUi() {
    setFixedSize(300, 220);
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
      "Left mouse<br>"
      "Right mouse<br>"
      "Wheel mouse<br>"
      "&lt;CTRL&gt; Left mouse<br>"
      "Space<br>"
      "R<br>"
      "F<br>"
      "X<br>"
      "Escape<br>"
      "?<br>"
      "</body></html>"
      );
    lblActions->setText(
      "<html><head/><body>"
      "Rotate<br>"
      "Translate<br>"
      "Zoom<br>"
      "Rotate Sun<br>"
      "Toggle projection<br>"
      "Rotate main model<br>"
      "Toggle fullscreen<br>"
      "Reset view<br>"
      "Quit<br>"
      "Show this help<br>"
      "</body></html>"
      );
  }
}
