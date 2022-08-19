#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QString>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define MAX_LEN 6   //定义值的最大
char get_str[100][30];
int get_num[100]={0};
//临时存储
//最终值
char show_str[100][30];
int show_num[100]={0};

#define num_MAX 60
QString Str_name[60];
int str_value[60];
int key[60]={0};
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("奇丑无比调参上位机");

    // 发送、接收计数清零
    sendNum = 0;
    recvNum = 0;
    // 状态栏
    QStatusBar *sBar = statusBar();
    // 状态栏的收、发计数标签
    lblSendNum = new QLabel(this);
    lblRecvNum = new QLabel(this);
    setNumOnLabel(lblSendNum, "S: ", sendNum);
    setNumOnLabel(lblRecvNum, "R: ", recvNum);
    // 从右往左依次添加
    sBar->addPermanentWidget(lblSendNum);
    sBar->addPermanentWidget(lblRecvNum);

    // 定时发送-定时器
    timSend = new QTimer;
    timSend->setInterval(1000);// 设置默认定时时长1000ms
    connect(timSend, &QTimer::timeout, this, [=](){
        on_btnSend_clicked();
    });

    // 新建一串口对象
    mySerialPort = new QSerialPort(this);

    // 串口接收，信号槽关联
    connect(mySerialPort, SIGNAL(readyRead()), this, SLOT(serialPortRead_Slot()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 绘图事件
void MainWindow::paintEvent(QPaintEvent *)
{
    // 绘图
    // 实例化画家对象，this指定绘图设备
    QPainter painter(this);

    // 设置画笔颜色
    QPen pen(QColor(0,0,0));
    // 设置画笔线宽（只对点线圆起作用，对文字不起作用）
    pen.setWidth(1);
    // 设置画笔线条风格，默认是SolidLine实线
    // DashLine虚线，DotLine点线，DashDotLine、DashDotDotLine点划线
    pen.setStyle(Qt::DashDotDotLine);
    // 让画家使用这个画笔
    painter.setPen(pen);

    painter.drawLine(QPoint(ui->txtRec->x() + ui->txtRec->width(), ui->txtRec->y()), QPoint(this->width(), ui->txtRec->y()));
    painter.drawLine(QPoint(ui->txtSend->x() + ui->txtSend->width(), ui->txtSend->y()), QPoint(this->width(), ui->txtSend->y()));
    painter.drawLine(QPoint(ui->statusbar->x(), ui->statusbar->y()-2), QPoint(this->width(), ui->statusbar->y()-2));

}

// 串口接收显示，槽函数
void MainWindow::serialPortRead_Slot()
{
    /*QString recBuf;
    recBuf = QString(mySerialPort->readAll());*/

    QByteArray recBuf;
    recBuf = mySerialPort->readAll();
//===============================================================================
    //参数刷新
    QString str;
    QString snum;
    int num;
    char buff[100] = {0};
    strcpy(buff, recBuf.toStdString().c_str());
    num=get_nowshow(buff,get_str,get_num,show_str,show_num);
    int rowline;
    rowline=ui->qlist1->rowCount();
    if(rowline<num){
        for(int j =rowline;j<num;j++){
            ui->qlist1->insertRow(rowline);
            ui->qlist1->setItem(rowline,0,new QTableWidgetItem(""));
            ui->qlist1->setItem(rowline,1,new QTableWidgetItem(""));
        }
    }
    for(int i=0;i<num;i++){
        str=QString::fromUtf8(show_str[i]);
        ui->qlist1->setItem(i,0,new QTableWidgetItem(str));
        snum=QString::number(show_num[i]);
        ui->qlist1->setItem(i,1,new QTableWidgetItem(snum));
    }
//==============================================================================================
    // 接收字节计数
    recvNum += recBuf.size();
    // 状态栏显示计数值
    setNumOnLabel(lblRecvNum, "R: ", recvNum);

    // 判断是否为16进制接收，将以后接收的数据全部转换为16进制显示（先前接收的部分在多选框槽函数中进行转换。最好多选框和接收区组成一个自定义控件，方便以后调用）
    if(ui->chkRec->checkState() == false){

        // 在当前位置插入文本，不会发生换行。如果没有移动光标到文件结尾，会导致文件超出当前界面显示范围，界面也不会向下滚动。
        ui->txtRec->insertPlainText(recBuf);
    }else{
        // 16进制显示，并转换为大写
        QString str1 = recBuf.toHex().toUpper();//.data();
        // 添加空格
        QString str2;
        for(int i = 0; i<str1.length (); i+=2)
        {
            str2 += str1.mid (i,2);
            str2 += " ";
        }
        ui->txtRec->insertPlainText(str2);
        //ui->txtRec->insertPlainText(recBuf.toHex());
    }

    // 移动光标到文本结尾
    ui->txtRec->moveCursor(QTextCursor::End);

    // 将文本追加到末尾显示，会导致插入的文本换行
    /*ui->txtRec->appendPlainText(recBuf);*/

    /*// 在当前位置插入文本，不会发生换行。如果没有移动光标到文件结尾，会导致文件超出当前界面显示范围，界面也不会向下滚动。
    ui->txtRec->insertPlainText(recBuf);
    ui->txtRec->moveCursor(QTextCursor::End);*/

    // 利用一个QString去获取消息框文本，再将新接收到的消息添加到QString尾部，但感觉效率会比当前位置插入低。也不会发生换行
    /*QString txtBuf;
    txtBuf = ui->txtRec->toPlainText();
    txtBuf += recBuf;
    ui->txtRec->setPlainText(txtBuf);
    ui->txtRec->moveCursor(QTextCursor::End);*/

    // 利用一个QString去缓存接收到的所有消息，效率会比上面高一点。但清空接收的时候，要将QString一并清空。
    /*static QString txtBuf;
    txtBuf += recBuf;
    ui->txtRec->setPlainText(txtBuf);
    ui->txtRec->moveCursor(QTextCursor::End);*/
}

// 打开/关闭串口 槽函数
void MainWindow::on_btnSwitch_clicked()
{

    QSerialPort::BaudRate baudRate;
    QSerialPort::DataBits dataBits;
    QSerialPort::StopBits stopBits;
    QSerialPort::Parity   checkBits;

    // 获取串口波特率
    // 有没有直接字符串转换为 int的方法？？？
    //baudRate = ui->cmbBaudRate->currentText().toInt();
    if(ui->cmbBaudRate->currentText() == "9600"){
        baudRate = QSerialPort::Baud9600;
    }else if(ui->cmbBaudRate->currentText() == "38400"){
        baudRate = QSerialPort::Baud38400;
    }else if(ui->cmbBaudRate->currentText() == "115200"){
        baudRate = QSerialPort::Baud115200;
    }else{

    }

    // 获取串口数据位
    if(ui->cmbData->currentText() == "5"){
        dataBits = QSerialPort::Data5;
    }else if(ui->cmbData->currentText() == "6"){
        dataBits = QSerialPort::Data6;
    }else if(ui->cmbData->currentText() == "7"){
        dataBits = QSerialPort::Data7;
    }else if(ui->cmbData->currentText() == "8"){
        dataBits = QSerialPort::Data8;
    }else{

    }

    // 获取串口停止位
    if(ui->cmbStop->currentText() == "1"){
        stopBits = QSerialPort::OneStop;
    }else if(ui->cmbStop->currentText() == "1.5"){
        stopBits = QSerialPort::OneAndHalfStop;
    }else if(ui->cmbStop->currentText() == "2"){
        stopBits = QSerialPort::TwoStop;
    }else{

    }

    // 获取串口奇偶校验位
    if(ui->cmbCheck->currentText() == "无"){
        checkBits = QSerialPort::NoParity;
    }else if(ui->cmbCheck->currentText() == "奇校验"){
        checkBits = QSerialPort::OddParity;
    }else if(ui->cmbCheck->currentText() == "偶校验"){
        checkBits = QSerialPort::EvenParity;
    }else{

    }

    // 想想用 substr strchr怎么从带有信息的字符串中提前串口号字符串
    // 初始化串口属性，设置 端口号、波特率、数据位、停止位、奇偶校验位数
    mySerialPort->setBaudRate(baudRate);
    mySerialPort->setDataBits(dataBits);
    mySerialPort->setStopBits(stopBits);
    mySerialPort->setParity(checkBits);
    //mySerialPort->setPortName(ui->cmbSerialPort->currentText());// 不匹配带有串口设备信息的文本
    // 匹配带有串口设备信息的文本
    QString spTxt = ui->cmbSerialPort->currentText();
    spTxt = spTxt.section(':', 0, 0);//spTxt.mid(0, spTxt.indexOf(":"));
    //qDebug() << spTxt;
    mySerialPort->setPortName(spTxt);

    // 根据初始化好的串口属性，打开串口
    // 如果打开成功，反转打开按钮显示和功能。打开失败，无变化，并且弹出错误对话框。
    if(ui->btnSwitch->text() == "打开串口"){
        if(mySerialPort->open(QIODevice::ReadWrite) == true){
            //QMessageBox::
            ui->btnSwitch->setText("关闭串口");
            // 让端口号下拉框不可选，避免误操作（选择功能不可用，控件背景为灰色）
            ui->cmbSerialPort->setEnabled(false);
            ui->cmbBaudRate->setEnabled(false);
            ui->cmbStop->setEnabled(false);
            ui->cmbData->setEnabled(false);
            ui->cmbCheck->setEnabled(false);
        }else{
            QMessageBox::critical(this, "错误提示", "串口打开失败！！！\r\n该串口可能被占用\r\n请选择正确的串口");
        }
    }else{
        mySerialPort->close();
        ui->btnSwitch->setText("打开串口");
        // 端口号下拉框恢复可选，避免误操作
        ui->cmbSerialPort->setEnabled(true);
        ui->cmbBaudRate->setEnabled(true);
        ui->cmbStop->setEnabled(true);
        ui->cmbData->setEnabled(true);
        ui->cmbCheck->setEnabled(true);
    }

}

// 发送按键槽函数
// 如果勾选16进制发送，按照asc2的16进制发送
void MainWindow::on_btnSend_clicked()
{
    QByteArray sendData;
    // 判断是否为16进制发送，将发送区全部的asc2转换为16进制字符串显示，发送的时候转换为16进制发送
    if(ui->chkSend->checkState() == false){
        // 字符串形式发送
        sendData = ui->txtSend->toPlainText().toLocal8Bit().data();
    }else{
        // 16进制发送
        sendData = QByteArray::fromHex(ui->txtSend->toPlainText().toUtf8()).data();
    }

    // 如发送成功，会返回发送的字节长度。失败，返回-1。
    int a = mySerialPort->write(sendData);
    // 发送字节计数并显示
    if(a > 0)
    {
        // 发送字节计数
        sendNum += a;
        // 状态栏显示计数值
        setNumOnLabel(lblSendNum, "S: ", sendNum);
    }

}
int DataParsingInt(const char *pBuff, const char *pLeft, const char *pRight, int *pRes)
{
    char *pBegin,*pEnd,*pTemp;
    pBegin = strstr(pBuff, pLeft);
    if (pBegin == NULL)
    {
        return -1;
    }
    else
    {
        pEnd = strstr(pBegin + strlen(pLeft), pRight);
        if (pEnd == NULL || pBegin == NULL || pBegin > pEnd)
        {
            return -1;
        } else {
            pBegin += strlen(pLeft);
            pTemp = (char *) malloc(pEnd - pBegin);
            memcpy(pTemp, pBegin, pEnd - pBegin);
            *pRes = atoi(pTemp);
            free(pTemp);
            return 0;
        }
    }
}
//入口data_tran   传入char的字符串,二级指针类型的值的名称,二级指针类型的get_pointer,总共变量个数
int data_tran(const char *get_string, const char **ptr_name, int **get_pointer, const unsigned len){
    for(int i=0;i<len;i++)
    {
        DataParsingInt(get_string, ptr_name[i], ";", get_pointer[i]);
    }
}
int get_data(const char *get_string,char turn_str[100][30],int *turn_num) {
    int i = 0;
    int go_flag = 0;
    char *begin, *end, pTemp[100];
    int flag = 0;
    int len = 0;
    char temp_str[100]={0};
    int temp_num=0;
    begin = strstr(get_string, "[");
    if (begin == NULL) {
        go_flag = 0;
        return -1;   //找不到[这个启始位 说明是不是这个数据
    } else {
        end = strstr(get_string, "]");
        if (end == NULL) {
            //找不到结尾 这个字符串也有问题
            go_flag = 0;
            return -1;
        } else {
            if (begin > end) {
                //再找一次
                end = strstr(end + 1, "]");
                if (end == NULL) {
                    return -1;   //找不到就返回错误
                    go_flag = 0;
                } else {
                    //go_flag = 1;  //找到了
                    // pTemp = (char *) malloc(end - begin);
                    memcpy(pTemp, begin+1, end - begin-1);
                }
            } else {
                //go_flag = 1;
                //pTemp = (char *) malloc(end - begin-1);
                memcpy(pTemp, begin+1, end - begin-1);
                // free(pTemp);//释放空间
                //说明找到了
            }
        }
    }
    i=0;
    int sum=0;
    flag=2;
    while (pTemp[len] != '\0')
        len++;
    while (i < len) {
        if (pTemp[i] == ':') {
            strcpy(turn_str[sum], temp_str);
            strcpy(temp_str, "");
            flag = 1;
        } else if (pTemp[i] == ';') {
            turn_num[sum] = temp_num;
            temp_num=0;
            flag = 2;//标志着重新开始,已经结束了
            sum++;
        } else{
            switch (flag) {
            case 2:
                strncat(temp_str, &pTemp[i],1);
                break;
            case 1:
                temp_num=temp_num*10+(pTemp[i]-'0');
                break;
            }
        }
        i++;
    }
    return sum;
}

int get_nowshow(const char *get_string,char str[100][30],int *str_num,char final_str[100][30],int *final_num) {
    int num= get_data(get_string,str,str_num);
    int j,i;
    static int rowline=0;
    for(i=0;i<num;i++)
    {
        for(j=0;j<rowline;j++){
            if(strcmp(str[i],final_str[j])==0)
                break;
        }
        if(j==rowline){
            //说明原字符串中没有这个数据
            strcpy(final_str[rowline],str[i]);  //拷贝数据
            if(final_num[rowline]!=str_num[i])
                final_num[rowline]=str_num[i];
            rowline++;
        }
        else{
            //说明已经有相同的数据了
            if(final_num[j]!=str_num[i])
                final_num[j]=str_num[i];
        }
    }
    return rowline;
}
// 状态栏标签显示计数值
void MainWindow::setNumOnLabel(QLabel *lbl, QString strS, long num)
{
    // 标签显示
    QString strN;
    strN.sprintf("%ld", num);
    QString str = strS + strN;
    lbl->setText(str);
}

void MainWindow::on_btnClearRec_clicked()
{
    ui->txtRec->clear();
    // 清除发送、接收字节计数
    sendNum = 0;
    recvNum = 0;
    // 状态栏显示计数值
    setNumOnLabel(lblSendNum, "S: ", sendNum);
    setNumOnLabel(lblRecvNum, "R: ", recvNum);
}

void MainWindow::on_btnClearSend_clicked()
{
    ui->txtSend->clear();
    // 清除发送字节计数
    sendNum = 0;
    // 状态栏显示计数值
    setNumOnLabel(lblSendNum, "S: ", sendNum);
}

// 先前接收的部分在多选框状态转换槽函数中进行转换。（最好多选框和接收区组成一个自定义控件，方便以后调用）
void MainWindow::on_chkRec_stateChanged(int arg1)
{
    // 获取文本字符串
    QString txtBuf = ui->txtRec->toPlainText();

    // 获取多选框状态，未选为0，选中为2
    // 为0时，多选框未被勾选，接收区先前接收的16进制数据转换为asc2字符串格式
    if(arg1 == 0){

        QByteArray str1 = QByteArray::fromHex(txtBuf.toUtf8());
        // 文本控件清屏，显示新文本
        ui->txtRec->clear();
        ui->txtRec->insertPlainText(str1);
        // 移动光标到文本结尾
        ui->txtRec->moveCursor(QTextCursor::End);

    }else{// 不为0时，多选框被勾选，接收区先前接收asc2字符串转换为16进制显示

        QByteArray str1 = txtBuf.toUtf8().toHex().toUpper();
        // 添加空格
        QByteArray str2;
        for(int i = 0; i<str1.length (); i+=2)
        {
            str2 += str1.mid (i,2);
            str2 += " ";
        }
        // 文本控件清屏，显示新文本
        ui->txtRec->clear();
        ui->txtRec->insertPlainText(str2);
        // 移动光标到文本结尾
        ui->txtRec->moveCursor(QTextCursor::End);

    }
}

// 先前发送区的部分在多选框状态转换槽函数中进行转换。（最好多选框和发送区组成一个自定义控件，方便以后调用）
void MainWindow::on_chkSend_stateChanged(int arg1)
{
    // 获取文本字符串
    QString txtBuf = ui->txtSend->toPlainText();

    // 获取多选框状态，未选为0，选中为2
    // 为0时，多选框未被勾选，将先前的发送区的16进制字符串转换为asc2字符串
    if(arg1 == 0){

        QByteArray str1 = QByteArray::fromHex(txtBuf.toUtf8());
        // 文本控件清屏，显示新文本
        ui->txtSend->clear();
        ui->txtSend->insertPlainText(str1);
        // 移动光标到文本结尾
        ui->txtSend->moveCursor(QTextCursor::End);

    }else{// 多选框被勾选，将先前的发送区的asc2字符串转换为16进制字符串

        QByteArray str1 = txtBuf.toUtf8().toHex().toUpper();
        // 添加空格
        QByteArray str2;
        for(int i = 0; i<str1.length (); i+=2)
        {
            str2 += str1.mid (i,2);
            str2 += " ";
        }
        // 文本控件清屏，显示新文本
        ui->txtSend->clear();
        ui->txtSend->insertPlainText(str2);
        // 移动光标到文本结尾
        ui->txtSend->moveCursor(QTextCursor::End);

    }
}

// 定时发送开关 选择复选框
void MainWindow::on_chkTimSend_stateChanged(int arg1)
{
    // 获取复选框状态，未选为0，选中为2
    if(arg1 == 0){
        timSend->stop();
        // 时间输入框恢复可选
        ui->txtSendMs->setEnabled(true);
    }else{
        // 对输入的值做限幅，小于10ms会弹出对话框提示
        if(ui->txtSendMs->text().toInt() >= 10){
            timSend->start(ui->txtSendMs->text().toInt());// 设置定时时长，重新计数
            // 让时间输入框不可选，避免误操作（输入功能不可用，控件背景为灰色）
            ui->txtSendMs->setEnabled(false);
        }else{
            ui->chkTimSend->setCheckState(Qt::Unchecked);
            QMessageBox::critical(this, "错误提示", "定时发送的最小间隔为 10ms\r\n请确保输入的值 >=10");
        }
    }
}

void MainWindow::on_pushButton_clicked()
{
    int rowline;
    rowline=ui->changelist->rowCount();
    ui->changelist->insertRow(rowline);
    ui->changelist->setItem(rowline,0,new QTableWidgetItem(""));
    ui->changelist->setItem(rowline,1,new QTableWidgetItem(""));
}
void MainWindow::on_pushButton_2_clicked()
{

    int rowIndex = ui->changelist->currentRow();
    if (rowIndex!=-1)
    {
        ui->changelist->removeRow(rowIndex);
    }


}

void MainWindow::on_pushButton_3_clicked()
{
    int num=0;
    int num_temp;
    int allrowline;
    QString str_temp,numstr_temp;
    QByteArray sendData;
    //首先要判断数据有没有变化
//表格数据获取部分
//============================================================================
    allrowline =ui->changelist->rowCount();
    //为确保不存在的值出现先遍历一遍
    num=0;
    for(int i=0;i<allrowline;i++){
        if( ((ui->changelist->item(i,0)) ==NULL) ||(ui->changelist->item(i,0)->text().isEmpty()))
        {
            ui->changelist->removeRow(i);
            i--;
            allrowline--;
        }
        else
        {
            if(((ui->changelist->item(i,1))==NULL)||(ui->changelist->item(i,1)->text().isEmpty()))
            {
                //防止出现指针问题
                ui->changelist->setItem(i,1,new QTableWidgetItem("0"));
            }
                str_temp = ui->changelist->item(i,0)->text();
                if(QString::compare(str_temp,Str_name[i])==0){
                    //内容相同
                    numstr_temp=ui->changelist->item(i,1)->text();
                    num_temp=numstr_temp.toInt();
                    if((str_value[i]==num_temp)){
                        //那就不用更新数据
                    }
                    else
                    {
                        //更新数据
                        key[num]=i;
                        num++;
                        str_value[i]=num_temp;
                    }
                }
                else{
                    //说明整个数据名称都不对应了那就换掉
                    numstr_temp=ui->changelist->item(i,1)->text();
                    num_temp=numstr_temp.toInt();
                    key[num]=i;
                    num++;
                    Str_name[i]=str_temp;
                    str_value[i]=num_temp;
                }
        }
    }
    //对后面行数数据清理
    for(int j=allrowline;j<num_MAX;j++){
        Str_name[j]="";
        str_value[j]=0;
    }//全部置零
//====================================================================================
    //组装数据
    QString get_str="";
    for(int k=0;k<num;k++){
        QString s = QString::number(str_value[key[k]]);
        get_str=get_str+Str_name[key[k]]+":"+s+";";
        key[k]=0;
    }
    qDebug("%s",qPrintable(get_str));
    sendData = get_str.toLatin1().data();
    int a = mySerialPort->write(sendData);
    if(a > 0)
    {
        // 发送字节计数
        sendNum += a;
        // 状态栏显示计数值
        setNumOnLabel(lblSendNum, "S: ", sendNum);
    }
}


void MainWindow::on_changelist_itemSelectionChanged()
{

}


void MainWindow::on_pushButton_4_clicked()
{
    QString str;
    QString snum;
    int num;
    char buff[100] = {0};
    strcpy(buff, "[a:50;e:20;c:30;abc:23;f:45;d:60;]");
    num=get_nowshow(buff,get_str,get_num,show_str,show_num);
    int rowline;
    rowline=ui->qlist1->rowCount();
    if(rowline<num){
        for(int j =rowline;j<num;j++){
            ui->qlist1->insertRow(rowline);
            ui->qlist1->setItem(rowline,0,new QTableWidgetItem(""));
            ui->qlist1->setItem(rowline,1,new QTableWidgetItem(""));
        }
    }
    for(int i=0;i<num;i++){
        str=QString::fromUtf8(show_str[i]);
        ui->qlist1->setItem(i,0,new QTableWidgetItem(str));
        snum=QString::number(show_num[i]);
        ui->qlist1->setItem(i,1,new QTableWidgetItem(snum));
    }

}


void MainWindow::on_pushButton_5_clicked()
{
    int num;
    QString str_save;
    int line_num=ui->changelist->rowCount();
    //为确保不存在的值出现先遍历一遍
    num=0;
    str_save="[";
    for(int i=0;i<line_num;i++){
        if( ((ui->changelist->item(i,0)) ==NULL) ||(ui->changelist->item(i,0)->text().isEmpty()))
        {
            ui->changelist->removeRow(i);
            i--;
            line_num--;
        }
        else
        {
            if(((ui->changelist->item(i,1))==NULL)||(ui->changelist->item(i,1)->text().isEmpty()))
            {
                //防止出现指针问题
                ui->changelist->setItem(i,1,new QTableWidgetItem("0"));
            }
            str_save+= ui->changelist->item(i,0)->text();
            str_save+=":";
            str_save+= ui->changelist->item(i,1)->text();
            str_save+=";";
        }
    }
    str_save+="]";
    QFile  myfile("data.config");//创建一个输出文件的文档
    if (myfile.open(QFile::WriteOnly|QFile::Truncate))//注意WriteOnly是往文本中写入的时候用，ReadOnly是在读文本中内容的时候用，Truncate表示将原来文件中的内容清空
    {
        //读取之前setPlainText的内容，或直接输出字符串内容QObject::tr()
        QTextStream out(&myfile);
        out<<str_save;
    }
    myfile.close();
}


void MainWindow::on_pushButton_6_clicked()
{
    char get_str_temp[100][30];
    int get_num_temp[100]={0};
    QString str_temp;
    QString snum_temp;
    //临时存储
    //最终值
    char show_str_temp[100][30];
    int show_num_temp[100]={0};
    QString str_read;
    QFile  myfile("data.config");//创建一个输出文件的文档
    if (myfile.open(QFile::ReadOnly|QFile::Text))//注意WriteOnly是往文本中写入的时候用，ReadOnly是在读文本中内容的时候用，Truncate表示将原来文件中的内容清空
    {
        //读取之前setPlainText的内容，或直接输出字符串内容QObject::tr()
        QTextStream out(&myfile);
        out>>str_read;
    }
    myfile.close();
    int num;
    char buff[100] = {0};
    strcpy(buff, str_read.toStdString().c_str());
    num=get_nowshow(buff,get_str_temp,get_num_temp,show_str_temp,show_num_temp);
    int rowline;
    rowline=ui->changelist->rowCount();
    if(rowline<num){
        for(int j =rowline;j<num;j++){
            ui->changelist->insertRow(rowline);
            ui->changelist->setItem(rowline,0,new QTableWidgetItem(""));
            ui->changelist->setItem(rowline,1,new QTableWidgetItem(""));
        }
    }
    for(int i=0;i<num;i++){
        str_temp=QString::fromUtf8(show_str_temp[i]);
        ui->changelist->setItem(i,0,new QTableWidgetItem(str_temp));
        snum_temp=QString::number(show_num_temp[i]);
        ui->changelist->setItem(i,1,new QTableWidgetItem(snum_temp));
    }
}

