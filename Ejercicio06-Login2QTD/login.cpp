#include "login.h"
#include "ui_login.h"

Login::Login(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Login)
{
    ui->setupUi(this);

    connect(ui->pbIngresar, SIGNAL(pressed()), this, SLOT(slot_validarUsuario()));
}

Login::~Login()
{
    delete ui;
}

void Login::slot_validarUsuario()
{

    if( ui->leUsuario->text() == "admin" && ui->leClave->text() == "1234") {
        this->close();

    }
    else {
        ui->leClave->clear();
    }


}
