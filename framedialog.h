#ifndef FRAMEDIALOG_H
#define FRAMEDIALOG_H

#include <QDialog>

namespace Ui {
class FrameDialog;
}

class FrameDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FrameDialog(QWidget *parent = 0);
    ~FrameDialog();

private:
    Ui::FrameDialog *ui;
public slots:
    void LoadFrame(QString path);
};

#endif // FRAMEDIALOG_H
