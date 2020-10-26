#include "finddialog.h"
#include "ui_finddialog.h"

#include <QDebug>

FindDialog::FindDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FindDialog),
    isCase(false),
    isLoop(false),
    directDown(true)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::WindowTitleHint |
                       Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);

    ui->radioButton_down->setChecked(true);

    QButtonGroup* btnGroup = new QButtonGroup();
    btnGroup->addButton(ui->radioButton_up,1);
    btnGroup->addButton(ui->radioButton_down,2);
    connect(btnGroup,SIGNAL(buttonClicked(int)),this,SLOT(slotDirectionSelect(int)));
}

FindDialog::~FindDialog()
{
    delete ui;
}

void FindDialog::setFindWindow(QString &reg)
{
    ui->widget_replace->setVisible(false);
    ui->pushButton_replace->setVisible(false);
    ui->pushButton_replaceAll->setVisible(false);

    ui->groupBox->setVisible(true);

    if(!reg.isNull() && !reg.isEmpty())
    {
        ui->lineEdit_find->clear();
        ui->lineEdit_find->setText(reg);
    }

    QString str = ui->lineEdit_find->text();
    if(str.isNull() || str.isEmpty())
    {
        ui->pushButton_find->setEnabled(false);
        ui->pushButton_replace->setEnabled(false);
        ui->pushButton_replaceAll->setEnabled(false);
    }
    this->setWindowTitle(QString::fromLocal8Bit("查找"));
    this->setVisible(true);
}

void FindDialog::setReplaceWindow(QString &reg)
{
    ui->groupBox->setVisible(false);

    ui->widget_replace->setVisible(true);
    ui->pushButton_replace->setVisible(true);
    ui->pushButton_replaceAll->setVisible(true);

    if(!reg.isNull() && !reg.isEmpty())
    {
        ui->lineEdit_find->clear();
        ui->lineEdit_find->setText(reg);
    }

    QString str = ui->lineEdit_find->text();
    if(str.isNull() || str.isEmpty())
    {
        ui->pushButton_find->setEnabled(false);
        ui->pushButton_replace->setEnabled(false);
        ui->pushButton_replaceAll->setEnabled(false);
    }
    ui->lineEdit_replace->clear();
    this->setWindowTitle(QString::fromLocal8Bit("替换"));
    this->setVisible(true);
}

void FindDialog::slotDirectionSelect(int id)
{
    if(id == 1)
        directDown = false;
    else
        directDown = true;
}

void FindDialog::on_checkBox_case_clicked(bool checked)
{
    isCase = checked;
}

void FindDialog::on_checkBox_loop_clicked(bool checked)
{
    isLoop = checked;
}

void FindDialog::on_lineEdit_find_textChanged(const QString &arg1)
{
    if(arg1.isNull() || arg1.isEmpty())
    {
        ui->pushButton_find->setEnabled(false);
        ui->pushButton_replace->setEnabled(false);
        ui->pushButton_replaceAll->setEnabled(false);
    }
    else
    {
        if(!ui->pushButton_find->isEnabled())
        {
            ui->pushButton_find->setEnabled(true);
            ui->pushButton_replace->setEnabled(true);
            ui->pushButton_replaceAll->setEnabled(true);
        }
    }
}

void FindDialog::on_pushButton_find_clicked()
{
    emit sigFindMode(ui->lineEdit_find->text().trimmed(),
                    isLoop,directDown,isCase);
}

void FindDialog::on_pushButton_replace_clicked()
{
    emit sigReplaceMode(ui->lineEdit_find->text().trimmed(),
                        ui->lineEdit_replace->text().trimmed(),
                        false,isLoop,isCase);
}

void FindDialog::on_pushButton_replaceAll_clicked()
{
    emit sigReplaceMode(ui->lineEdit_find->text().trimmed(),
                        ui->lineEdit_replace->text().trimmed(),
                        true,isLoop,isCase);
}

void FindDialog::on_pushButton_cancle_clicked()
{
    this->close();
}
