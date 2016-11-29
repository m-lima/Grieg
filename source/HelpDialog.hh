#ifndef HELPUH7620_H
#define HELPUH7620_H

#include <QtCore/QVariant>
#include <QDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>

namespace View {
  class HelpDialog : public QDialog {
    Q_OBJECT

  public:
    HelpDialog(QWidget *parent = 0);
    ~HelpDialog() = default;

  private:
    QVBoxLayout *lytMain;
    QHBoxLayout *lytTexts;
    QLabel *lblKeys;
    QLabel *lblActions;
    QDialogButtonBox *buttonBox;

    void setupUi();
    void retranslateUi();

  };
}

#endif // HELPUH7620_H