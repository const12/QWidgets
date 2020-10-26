#include "notepad.h"
#include "ui_notepad.h"

#include <QDebug>
#include <QMessageBox>
#include <QApplication>
#include <QFileDialog>
#include <QStandardPaths>
#include <QFile>
#include <QDateTime>
#include <QTextCodec>
#include <QMimeDatabase>

NotePad::NotePad(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::NotePad)
{
    ui->setupUi(this);
    qApp->setApplicationName(m_appName);

    initWidgets();
    initConnections();
}

NotePad::~NotePad()
{
    delete ui;
}

void NotePad::closeEvent(QCloseEvent *event)
{
    bool accept = true;
    if(m_isModity)
    {
        int res = warningMoreButton(QString(),
                                    QString::fromLocal8Bit("尚未保存，是否将更改保存至 %1 ?")
                                    .arg(getFileName()),
                                    QString::fromLocal8Bit("保存"),QString::fromLocal8Bit("不保存"));
        if(res == QMessageBox::Cancel)
            accept = false;
        else if(res == QMessageBox::Yes)
        {
            if(!action_saveFile())
                accept = false;
        }
    }

    if(accept)
        event->accept();
    else
        event->ignore();
}

void NotePad::initWidgets()
{
    m_isModity = false;
    m_filePath = m_noneFilename;
    setWindowTitle(combineWindowTitle());

    ui->action_undo->setEnabled(false);
    ui->action_copy->setEnabled(false);
    ui->action_trim->setEnabled(false);
    ui->action_del->setEnabled(false);

    m_perInfo = new QLabel(this);
    m_perInfo->setStyleSheet("font: 9pt '微软雅黑';color: #808080;");
    m_perInfo->setText(QString("<span>Copyright &copy; 2020 TextEdit</span>"));
    ui->statusBar->addPermanentWidget(m_perInfo);

    m_findDlg = new FindDialog(this);
    m_findDlg->setVisible(false);
    connect(m_findDlg,SIGNAL(sigFindMode(QString,bool,bool,bool)),this,SLOT(slotFindMode(QString,bool,bool,bool)));
    connect(m_findDlg,SIGNAL(sigReplaceMode(QString&,QString&,bool,bool,bool)),this,SLOT(slotReplaceMode(QString&,QString&,bool,bool,bool)));
}

void NotePad::initConnections()
{
    connect(ui->plainTextEdit,SIGNAL(textChanged()),this,SLOT(slotTextChange()));
    connect(ui->plainTextEdit,SIGNAL(undoAvailable(bool)),ui->action_undo,SLOT(setEnabled(bool)));
    connect(ui->plainTextEdit,SIGNAL(copyAvailable(bool)),ui->action_copy,SLOT(setEnabled(bool)));
    connect(ui->plainTextEdit,SIGNAL(copyAvailable(bool)),ui->action_trim,SLOT(setEnabled(bool)));
    connect(ui->plainTextEdit,SIGNAL(copyAvailable(bool)),ui->action_del,SLOT(setEnabled(bool)));

    connect(ui->action_undo,SIGNAL(triggered(bool)),ui->plainTextEdit,SLOT(undo()));
    connect(ui->action_trim,SIGNAL(triggered(bool)),ui->plainTextEdit,SLOT(cut()));
    connect(ui->action_copy,SIGNAL(triggered(bool)),ui->plainTextEdit,SLOT(copy()));
    connect(ui->action_paste,SIGNAL(triggered(bool)),ui->plainTextEdit,SLOT(paste()));
    connect(ui->action_all,SIGNAL(triggered(bool)),ui->plainTextEdit,SLOT(selectAll()));

    connect(ui->action_new,SIGNAL(triggered(bool)),this,SLOT(action_newFile()));
    connect(ui->action_open,SIGNAL(triggered(bool)),this,SLOT(action_openFile()));
    connect(ui->action_close,SIGNAL(triggered(bool)),this,SLOT(action_closeFile()));
    connect(ui->action_save,SIGNAL(triggered(bool)),this,SLOT(action_saveFile()));
    connect(ui->action_saveAs,SIGNAL(triggered(bool)),this,SLOT(action_saveAsOther()));
    connect(ui->action_exit,SIGNAL(triggered(bool)),this,SLOT(action_exitApp()));

    connect(ui->action_del,SIGNAL(triggered(bool)),this,SLOT(action_del()));
    connect(ui->action_find,SIGNAL(triggered(bool)),this,SLOT(action_find()));
    connect(ui->action_replace,SIGNAL(triggered(bool)),this,SLOT(action_replace()));
    connect(ui->action_date,SIGNAL(triggered(bool)),this,SLOT(action_addTime()));
    connect(ui->action_about,SIGNAL(triggered(bool)),this,SLOT(action_about()));
}

inline const QString NotePad::combineWindowTitle()
{
    QString title = getFileName() + m_titlwConn + m_appName;
    return title;
}

inline const QString NotePad::getFileName()
{
    return QFileInfo(m_filePath).fileName();
}

inline void NotePad::errorInfoDisplay(QString& info)
{
    QMessageBox box(QMessageBox::Critical,
                    m_appName,info,
                    QMessageBox::Ok);
    box.setButtonText(QMessageBox::Ok,QString::fromLocal8Bit("确定"));
    box.setMinimumSize(150,450);
    box.exec();
}

inline void NotePad::wariningInfoDisplay(QString &info)
{
    QMessageBox box(QMessageBox::Warning,
                    m_appName,info,
                    QMessageBox::Ok);
    box.setButtonText(QMessageBox::Ok,QString::fromLocal8Bit("确定"));
    box.setMinimumSize(150,450);
    box.exec();
}

inline int NotePad::warningStandButton(QString &title, QString &alert, QString yes, QString no)
{
    if(title.isEmpty())
        title = m_appName;

    QMessageBox box(QMessageBox::Warning,title,alert,
                    QMessageBox::Yes | QMessageBox::No);
    box.setButtonText(QMessageBox::Yes,yes);
    box.setButtonText(QMessageBox::No,no);

    return box.exec();
}

inline int NotePad::warningMoreButton(QString &title, QString &alert, QString yes, QString no, QString cancle)
{
    if(title.isEmpty())
        title = m_appName;

    QMessageBox box(QMessageBox::Warning,title,alert,
                    QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    box.setButtonText(QMessageBox::Yes,yes);
    box.setButtonText(QMessageBox::No,no);
    box.setButtonText(QMessageBox::Cancel,cancle);

    return box.exec();
}

bool NotePad::fileSave(QString data)
{
    if(data.isEmpty())
    {
        data = ui->plainTextEdit->document()->toPlainText().trimmed();
//        if(data.isEmpty())
//        {
//            errorInfoDisplay(QString::fromLocal8Bit("文本信息不得为空！"));
//            return;
//        }
    }

    if(m_filePath.isNull() || m_filePath.isEmpty() ||
            !QFileInfo(m_filePath).dir().exists())
    {
        errorInfoDisplay(QString::fromLocal8Bit("文件路径不存在！"));
        return false;
    }

    QFile file(m_filePath);
    if(file.open(QIODevice::ReadWrite))// | QFile::Text
    {
        // 鼠标指针变为等待状态
        QApplication::setOverrideCursor(Qt::WaitCursor);
        int res = file.write(data.toStdString().c_str());
        file.close();
        // 鼠标指针恢复原来的状态
        QApplication::restoreOverrideCursor();

        if(res < 0)
        {
            errorInfoDisplay(QString::fromLocal8Bit("信息写入失误:")
                             .append(file.errorString()));
            return false;
        }

        m_isModity = false;
        setWindowTitle(combineWindowTitle());
        return true;
    }
    errorInfoDisplay(QString::fromLocal8Bit("文件打开失败！"));
    return false;
}

bool NotePad::loadFile(QString &fileName)
{
    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForFile(fileName);
    if(!mime.name().startsWith("text/"))
    {
        int ret = warningStandButton(QString(),
                                     QString::fromLocal8Bit("不合适的文档类型:%1，是否打开文件？")
                                     .arg(mime.name()),
                                     QString::fromLocal8Bit("坚持打开"),
                                     QString::fromLocal8Bit("取消"));
        if(ret != QMessageBox::Yes)
            return false;
    }

    QFile file(fileName); // 新建QFile对象
    if (!file.open(QFile::ReadOnly)) {
        errorInfoDisplay(QString::fromLocal8Bit("文件读取失败！")
                         .append("\nError:").append(file.errorString()));
        return false;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);


    QTextStream in(&file);
    in.setCodec(QTextCodec::codecForName("UTF-8"));
    ui->plainTextEdit->setPlainText(in.readAll());

    file.close();
    QApplication::restoreOverrideCursor();

    m_isModity = false;
    m_filePath = fileName;
    setWindowTitle(combineWindowTitle());

    return true;
}

inline void NotePad::resetWindowUI()
{
    m_isModity = false;
    if(m_filePath != m_noneFilename)
    {
        m_filePath = m_noneFilename;
        setWindowTitle(combineWindowTitle());
    }

    ui->plainTextEdit->blockSignals(true);
    ui->plainTextEdit->clear();
    ui->plainTextEdit->setVisible(true);
    ui->plainTextEdit->blockSignals(false);
}

void NotePad::textEditCursorEndToEnd(bool down)
{//param : 向下查找
    QTextCursor cursor = ui->plainTextEdit->textCursor();
    if(down )//&& cursor.atEnd()
    {
        cursor.setPosition(0);
        ui->plainTextEdit->setTextCursor(cursor);
    }

    if(!down)// && cursor.atStart()
    {
        cursor.setPosition(ui->plainTextEdit->document()->toPlainText().length());
        ui->plainTextEdit->setTextCursor(cursor);
    }
}

/*--------------------------------------------------*/
void NotePad::action_newFile()
{
    if(!m_isModity)
    {
        resetWindowUI();
    }
    else
    {
        int res = warningMoreButton(QString(),
                                    QString::fromLocal8Bit("尚未保存，是否将更改保存至 %1 ?")
                                    .arg(getFileName()),
                                    QString::fromLocal8Bit("保存"),QString::fromLocal8Bit("不保存"));
        if(res == QMessageBox::Yes)
        {
            if(action_saveFile())
                resetWindowUI();
        }
        else if(res == QMessageBox::No)
        {
            resetWindowUI();
        }
        else if(res == QMessageBox::Cancel)
        {}
    }
}

void NotePad::action_openFile()
{
    if(m_isModity)
    {
        int res = warningMoreButton(QString(),
                                    QString::fromLocal8Bit("尚未保存，是否将更改保存至 %1 ?")
                                    .arg(getFileName()),
                                    QString::fromLocal8Bit("保存"),QString::fromLocal8Bit("不保存"));
        if(res == QMessageBox::Cancel)
            return;
        else if(res == QMessageBox::Yes)
        {
            if(!action_saveFile())
                return;
        }
    }

    QString fileName = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("打开"),
                                                    QStandardPaths::displayName(QStandardPaths::DocumentsLocation),
                                                    tr("Text (*.txt);;All (*.*)"));
    if(fileName.isNull() || fileName.isEmpty())
        return;
    loadFile(fileName);
    ui->plainTextEdit->blockSignals(true);
    ui->plainTextEdit->setVisible(true);
    ui->plainTextEdit->blockSignals(false);
}

void NotePad::action_closeFile()
{
    if(m_isModity)
    {
        int res = warningMoreButton(QString(),
                                    QString::fromLocal8Bit("尚未保存，是否将更改保存至 %1 ?")
                                    .arg(getFileName()),
                                    QString::fromLocal8Bit("保存"),QString::fromLocal8Bit("不保存"));
        if(res == QMessageBox::Cancel)
            return;
        else if(res == QMessageBox::Yes)
        {
            if(!action_saveFile())
                return;
        }
    }

    resetWindowUI();
    ui->plainTextEdit->blockSignals(true);
    ui->plainTextEdit->setVisible(false);
    ui->plainTextEdit->blockSignals(false);
}

bool NotePad::action_saveFile()
{
    if(m_filePath == m_noneFilename)
        return action_saveAsOther();
    return fileSave();
}

bool NotePad::action_saveAsOther()
{
    QString fileName = QFileDialog::getSaveFileName(this, QString::fromLocal8Bit("另存为"),
                               QStandardPaths::displayName(QStandardPaths::DocumentsLocation),
                               tr("Text (*.txt);;All (*.*)"));
    //tr("Images (*.png *.xpm *.jpg);;Text files (*.txt);;XML files (*.xml)"));

    if(!fileName.isEmpty() && !fileName.isNull())
    {
        m_filePath = fileName;
        return fileSave();
    }
    return false;
}

void NotePad::action_exitApp()
{
    if(m_isModity)
    {
        int res = warningMoreButton(QString(),
                                    QString::fromLocal8Bit("尚未保存，是否将更改保存至 %1 ?")
                                    .arg(getFileName()),
                                    QString::fromLocal8Bit("保存"),QString::fromLocal8Bit("不保存"));
        if(res == QMessageBox::Cancel)
            return;
        else if(res == QMessageBox::Yes)
        {
            if(!action_saveFile())
                return;
        }
    }
    qApp->exit(0);
}

void NotePad::action_del()
{
    ui->plainTextEdit->textCursor().clearSelection();
    ui->plainTextEdit->insertPlainText("");
}

void NotePad::action_find()
{
    m_findDlg->setFindWindow(ui->plainTextEdit->textCursor().selectedText());
}

void NotePad::action_replace()
{
    m_findDlg->setReplaceWindow(ui->plainTextEdit->textCursor().selectedText());
}

void NotePad::action_addTime()
{
    QString time = QDateTime::currentDateTime().toString("HH:mm yyyy/MM/dd");
    ui->plainTextEdit->insertPlainText(time);
}

void NotePad::action_about()
{
    QMessageBox::about(this,m_appName,
                       QString::fromLocal8Bit("本应用简单示例QMainwidow的具体结构："
                                              "标题栏、菜单栏、工具栏状态栏、活动窗口以及中心窗口。"));
}

/*--------------------------------------------------*/
void NotePad::slotTextChange()
{
    if(!m_isModity)
        setWindowTitle("*" + windowTitle());
    m_isModity = true;
}

void NotePad::slotFindMode(QString reg, bool isLoop, bool dirDown, bool isCase)
{
    int flag = 0;
    if(!dirDown)
        flag |= QTextDocument::FindBackward;
    if(isCase)
        flag |= QTextDocument::FindCaseSensitively;

    if(!ui->plainTextEdit->find(reg,(QTextDocument::FindFlag)flag))
    {
        if(isLoop)
        {
            textEditCursorEndToEnd(dirDown);
            if(!ui->plainTextEdit->find(reg,(QTextDocument::FindFlag)flag))
                wariningInfoDisplay(QString::fromLocal8Bit("未找到 ").append(reg));
        }
        else
        {
            wariningInfoDisplay(QString::fromLocal8Bit("未找到 ").append(reg));
        }
    }
}

void NotePad::slotReplaceMode(QString &source, QString &con, bool all, bool isLoop, bool isCase)
{
    int flag = 0;
    if(isCase)
        flag |= QTextDocument::FindCaseSensitively;

    bool alert = false;
    do{
        if(ui->plainTextEdit->textCursor().selectedText() == source)
        {
            ui->plainTextEdit->textCursor().clearSelection();
            ui->plainTextEdit->insertPlainText(con);
        }

        if(!ui->plainTextEdit->find(source,(QTextDocument::FindFlag)flag))
        {
            alert = true;
            if(all || isLoop)
            {
                textEditCursorEndToEnd(true);
                alert = !ui->plainTextEdit->find(source,(QTextDocument::FindFlag)flag);
            }

            if(all && alert)
                break;
        }
    }while(all);

    if(alert)
        wariningInfoDisplay(QString::fromLocal8Bit("替换已完成"));
}
