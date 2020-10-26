#ifndef NOTEPAD_H
#define NOTEPAD_H

#include <QMainWindow>
#include <QDialog>
#include <QLabel>
#include "finddialog.h"

namespace Ui {
class NotePad;
}

enum Status{
    NoDone = 0,
    Done,
    Cancle
};
/**
 * 2020/12/26
 * @brief The NotePad class
 */
class NotePad : public QMainWindow
{
    Q_OBJECT

public:
    explicit NotePad(QWidget *parent = 0);
    ~NotePad();

private:
    Ui::NotePad *ui;
    const QString m_appName = QString::fromLocal8Bit("文本编辑器");
    const QString m_titlwConn = QString::fromLocal8Bit(" - ");
    const QString m_noneFilename = QString::fromLocal8Bit("未命名");

    bool    m_isModity;
    QString m_filePath;

    FindDialog* m_findDlg;
    QLabel*     m_perInfo;

protected:
    void    closeEvent(QCloseEvent* event);

private:
    void    initWidgets();
    void    initConnections();
    const   QString combineWindowTitle();
    const   QString getFileName();
    void    resetWindowUI();
    void    textEditCursorEndToEnd(bool down);

    void    errorInfoDisplay(QString& info);
    void    wariningInfoDisplay(QString& info);
    int     warningStandButton(QString& title,QString& alert,QString yes = QString::fromLocal8Bit("确定"),QString no = QString::fromLocal8Bit("取消"));
    int     warningMoreButton(QString& title,QString& alert,QString yes,QString no,QString cancle = QString::fromLocal8Bit("取消"));

    bool    fileSave(QString data = "");
    bool    loadFile(QString& filename);

private slots:
    void    action_newFile();
    void    action_openFile();
    void    action_closeFile();
    bool    action_saveFile();
    bool    action_saveAsOther();
    void    action_exitApp();

    void    action_del();
    void    action_find();
    void    action_replace();
    void    action_addTime();

    void    action_about();

    void    slotTextChange();
    void    slotFindMode(QString reg,bool isLoop,bool dirDown,bool isCase);
    void    slotReplaceMode(QString& source,QString& con,
                            bool all,bool isLoop,bool isCase);
};

#endif // NOTEPAD_H
