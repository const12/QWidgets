#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>

namespace Ui {
class FindDialog;
}

class FindDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindDialog(QWidget *parent = 0);
    ~FindDialog();

    void    setFindWindow(QString& reg = QString(""));
    void    setReplaceWindow(QString& reg = QString(""));

private:
    Ui::FindDialog *ui;
    bool isCase;
    bool isLoop;
    bool directDown;

private slots:
    void    slotDirectionSelect(int id);

    void    on_checkBox_case_clicked(bool checked);
    void    on_checkBox_loop_clicked(bool checked);
    void    on_lineEdit_find_textChanged(const QString &arg1);

    void    on_pushButton_find_clicked();
    void    on_pushButton_replace_clicked();
    void    on_pushButton_replaceAll_clicked();
    void    on_pushButton_cancle_clicked();

signals:
    void    sigFindMode(QString reg,bool isLoop,bool directDown,bool isCasee);
    void    sigReplaceMode(QString& source,QString&,bool isAll,bool isLoop,bool isCasee);
};

#endif // FINDDIALOG_H
