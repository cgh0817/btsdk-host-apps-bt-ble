/*
 * Copyright 2019, Cypress Semiconductor Corporation or a subsidiary of
 * Cypress Semiconductor Corporation. All Rights Reserved.
 *
 * This software, including source code, documentation and related
 * materials ("Software"), is owned by Cypress Semiconductor Corporation
 * or one of its subsidiaries ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products. Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 */

/*
 * Sample MCU application for BLE or BR/EDR HID Device using WICED HCI protocol.
 */

#include "app_include.h"
#include "usb_kb_usage.h"
#include "hci_control_api.h"

extern "C"
{
#include "app_host_hidd.h"
}

// Initialize app
void MainWindow::InitBLEHIDD()
{
    m_pairing_mode = 0;
    m_host_valid = false;
    m_b_is_hidd = false;
    ui->cbBLEHIDInterupt->clear();
    ui->cbBLEHIDReport->clear();

    ui->cbBLEHIDInterupt->addItem("Control Channel", HCI_CONTROL_HID_REPORT_CHANNEL_CONTROL);
    ui->cbBLEHIDInterupt->addItem("Interrupt Channel", HCI_CONTROL_HID_REPORT_CHANNEL_INTERRUPT);

    ui->cbBLEHIDReport->addItem("Other", HCI_CONTROL_HID_REPORT_TYPE_OTHER);
    ui->cbBLEHIDReport->addItem("Input", HCI_CONTROL_HID_REPORT_TYPE_INPUT);
    ui->cbBLEHIDReport->addItem("Output", HCI_CONTROL_HID_REPORT_TYPE_OUTPUT);
    ui->cbBLEHIDReport->addItem("Feature", HCI_CONTROL_HID_REPORT_TYPE_FEATURE);

    ui->cbBLEHIDInterupt->setCurrentIndex(1);
    ui->cbBLEHIDReport->setCurrentIndex(1);
    btnBLEHIDSendKeyInit();
    UpdateHIDD_ui_host();
    UpdateHIDD_ui_pairing();
}

void MainWindow::btnBLEHIDSendKeyInit()
{
    memset(keyRpt_buf, 0, KEYRPT_BUF_SIZE);
    keyRpt_buf[0] = HCI_CONTROL_HID_REPORT_CHANNEL_INTERRUPT;
    keyRpt_buf[1] = HCI_CONTROL_HID_REPORT_TYPE_INPUT;
    keyRpt_buf[2] = HCI_CONTROL_HID_REPORT_ID;
    ui->cbBLEHIDCapLock->setChecked(false);
    ui->cbBLEHIDCtrl->setChecked(false);
    ui->cbBLEHIDAlt->setChecked(false);
}

void MainWindow::btnBLEHIDSendKey()
{
    Log("Send key report: RptId:%02x Modifier:%02x %02x Keys:%02x %02x %02x %02x %02x %02x",
       keyRpt_buf[2], keyRpt_buf[3], keyRpt_buf[4], keyRpt_buf[5], keyRpt_buf[6], keyRpt_buf[7], keyRpt_buf[8], keyRpt_buf[9], keyRpt_buf[10]);
    wiced_hci_send_command(HCI_CONTROL_HIDD_COMMAND_SEND_REPORT, keyRpt_buf, KEYRPT_SIZE);
}

void MainWindow::btnBLEHIDSendKeyDown(BYTE c, QPushButton * button)
{
    bool release = ui->cbBLEHIDHold->isChecked() && btnBLEHIDSendKeyRelease(c, button);
    bool send = release;

    if (!release)
    {

        for (int i=KEYRPT_CODE;i<KEYRPT_SIZE;i++)
        {
            // if empty, then use it
            if (!keyRpt_buf[i])
            {
                keyRpt_buf[i] = c;
                setHIDD_buttonColor(button, Qt::blue);
                send = true;
                break;
            }
        }
    }

    if (send)
        btnBLEHIDSendKey();
}

bool MainWindow::btnBLEHIDSendKeyRelease(BYTE c, QPushButton * button)
{
    for (int i=KEYRPT_CODE;i<KEYRPT_SIZE;i++)
    {
        // if found the key, then shift it
        if (keyRpt_buf[i]==c)
        {
            setHIDD_buttonColor(button, Qt::white);
            do {
                keyRpt_buf[i] = keyRpt_buf[i+1];
            } while (keyRpt_buf[i] && ++i < KEYRPT_SIZE);
            return true;
        }
    }
    return false;
}

void MainWindow::btnBLEHIDSendKeyUp(BYTE c, QPushButton * button)
{
    if (!ui->cbBLEHIDHold->isChecked())
    {
        btnBLEHIDSendKeyRelease(c, button);
        btnBLEHIDSendKey();
    }
}

void MainWindow::on_cbBLEHIDHold_clicked()
{
    if (!ui->cbBLEHIDHold->isChecked())
    {
        bool sendRpt;

        sendRpt = btnBLEHIDSendKeyRelease(USB_USAGE_1, ui->btnBLEHIDSendKey_1);
        sendRpt |= btnBLEHIDSendKeyRelease(USB_USAGE_2, ui->btnBLEHIDSendKey_2);
        sendRpt |= btnBLEHIDSendKeyRelease(USB_USAGE_3, ui->btnBLEHIDSendKey_3);
        sendRpt |= btnBLEHIDSendKeyRelease(USB_USAGE_A, ui->btnBLEHIDSendKey_a);
        sendRpt |= btnBLEHIDSendKeyRelease(USB_USAGE_B, ui->btnBLEHIDSendKey_b);
        sendRpt |= btnBLEHIDSendKeyRelease(USB_USAGE_C, ui->btnBLEHIDSendKey_c);
        sendRpt |= btnBLEHIDSendKeyRelease(USB_USAGE_ENTER, ui->btnBLEHIDSendKey_enter);
        sendRpt |= btnBLEHIDSendKeyRelease(USB_USAGE_ESCAPE, ui->btnBLEHIDSendKey_esc);
        sendRpt |= btnBLEHIDSendKeyRelease(USB_USAGE_DOWN_ARROW, ui->btnBLEHIDSendKey_down);
        sendRpt |= btnBLEHIDSendKeyRelease(USB_USAGE_UP_ARROW, ui->btnBLEHIDSendKey_up);
        sendRpt |= btnBLEHIDSendKeyRelease(USB_USAGE_LEFT_ARROW, ui->btnBLEHIDSendKey_left);
        sendRpt |= btnBLEHIDSendKeyRelease(USB_USAGE_RIGHT_ARROW, ui->btnBLEHIDSendKey_right);
        if (sendRpt)
            btnBLEHIDSendKey();
    }
}

void MainWindow::on_btnBLEHIDSendKey_1_pressed()
{
    btnBLEHIDSendKeyDown(USB_USAGE_1, ui->btnBLEHIDSendKey_1);
}

void MainWindow::on_btnBLEHIDSendKey_1_released()
{
    btnBLEHIDSendKeyUp(USB_USAGE_1, ui->btnBLEHIDSendKey_1);
}

void MainWindow::on_btnBLEHIDSendKey_2_pressed()
{
    btnBLEHIDSendKeyDown(USB_USAGE_2, ui->btnBLEHIDSendKey_2);
}

void MainWindow::on_btnBLEHIDSendKey_2_released()
{
    btnBLEHIDSendKeyUp(USB_USAGE_2, ui->btnBLEHIDSendKey_2);
}

void MainWindow::on_btnBLEHIDSendKey_3_pressed()
{
    btnBLEHIDSendKeyDown(USB_USAGE_3, ui->btnBLEHIDSendKey_3);
}

void MainWindow::on_btnBLEHIDSendKey_3_released()
{
    btnBLEHIDSendKeyUp(USB_USAGE_3, ui->btnBLEHIDSendKey_3);
}

void MainWindow::on_btnBLEHIDSendKey_a_pressed()
{
    btnBLEHIDSendKeyDown(USB_USAGE_A, ui->btnBLEHIDSendKey_a);
}

void MainWindow::on_btnBLEHIDSendKey_a_released()
{
    btnBLEHIDSendKeyUp(USB_USAGE_A, ui->btnBLEHIDSendKey_a);
}

void MainWindow::on_btnBLEHIDSendKey_b_pressed()
{
    btnBLEHIDSendKeyDown(USB_USAGE_B, ui->btnBLEHIDSendKey_b);
}

void MainWindow::on_btnBLEHIDSendKey_b_released()
{
    btnBLEHIDSendKeyUp(USB_USAGE_B, ui->btnBLEHIDSendKey_b);
}

void MainWindow::on_btnBLEHIDSendKey_c_pressed()
{
    btnBLEHIDSendKeyDown(USB_USAGE_C, ui->btnBLEHIDSendKey_c);
}

void MainWindow::on_btnBLEHIDSendKey_c_released()
{
    btnBLEHIDSendKeyUp(USB_USAGE_C, ui->btnBLEHIDSendKey_c);
}

void MainWindow::on_btnBLEHIDSendKey_esc_pressed()
{
    btnBLEHIDSendKeyDown(USB_USAGE_ESCAPE, ui->btnBLEHIDSendKey_esc);
}

void MainWindow::on_btnBLEHIDSendKey_esc_released()
{
    btnBLEHIDSendKeyUp(USB_USAGE_ESCAPE, ui->btnBLEHIDSendKey_esc);
}

void MainWindow::on_btnBLEHIDSendKey_up_pressed()
{
    btnBLEHIDSendKeyDown(USB_USAGE_UP_ARROW, ui->btnBLEHIDSendKey_up);
}

void MainWindow::on_btnBLEHIDSendKey_up_released()
{
    btnBLEHIDSendKeyUp(USB_USAGE_UP_ARROW, ui->btnBLEHIDSendKey_up);
}

void MainWindow::on_btnBLEHIDSendKey_enter_pressed()
{
    btnBLEHIDSendKeyDown(USB_USAGE_ENTER, ui->btnBLEHIDSendKey_enter);
}

void MainWindow::on_btnBLEHIDSendKey_enter_released()
{
    btnBLEHIDSendKeyUp(USB_USAGE_ENTER, ui->btnBLEHIDSendKey_enter);
}

void MainWindow::on_btnBLEHIDSendKey_left_pressed()
{
    btnBLEHIDSendKeyDown(USB_USAGE_LEFT_ARROW, ui->btnBLEHIDSendKey_left);
}

void MainWindow::on_btnBLEHIDSendKey_left_released()
{
    btnBLEHIDSendKeyUp(USB_USAGE_LEFT_ARROW, ui->btnBLEHIDSendKey_left);
}

void MainWindow::on_btnBLEHIDSendKey_down_pressed()
{
    btnBLEHIDSendKeyDown(USB_USAGE_DOWN_ARROW, ui->btnBLEHIDSendKey_down);
}

void MainWindow::on_btnBLEHIDSendKey_down_released()
{
    btnBLEHIDSendKeyUp(USB_USAGE_DOWN_ARROW, ui->btnBLEHIDSendKey_down);
}

void MainWindow::on_btnBLEHIDSendKey_right_pressed()
{
    btnBLEHIDSendKeyDown(USB_USAGE_RIGHT_ARROW, ui->btnBLEHIDSendKey_right);
}

void MainWindow::on_btnBLEHIDSendKey_right_released()
{
    btnBLEHIDSendKeyUp(USB_USAGE_RIGHT_ARROW, ui->btnBLEHIDSendKey_right);
}

void MainWindow::setHIDD_buttonColor(QPushButton * button, const QColor &color)
{
    QPalette pal = button->palette();
    pal.setColor(QPalette::Button, color);
    button->setPalette(pal);
    button->setAutoFillBackground(true);
    button->update();
}

void MainWindow::setHIDD_HostAddr(unsigned char * ad)
{
    m_host_valid = (bool) ad;
    if (m_host_valid)
    {
        for (int i=0;i<6;i++)
            m_host_ad[i]= *ad++;
    }
    UpdateHIDD_ui_host();
}

void MainWindow::setHIDD_linkChange(unsigned char * ad, bool linkUp)
{
    m_connected = linkUp;
    if (linkUp)
    {
        m_host_valid = (bool) ad;
        if (m_host_valid)
        {
            for (int i=0;i<6;i++)
                m_host_ad[i]= *ad++;
        }
        ui->btnBLEHIDConnectDisconnect->setText("Disconnect");
    }
    else
    {
        ui->btnBLEHIDConnectDisconnect->setText("Connect");
    }
    UpdateHIDD_ui_host();
}

// Connect to peer device
void MainWindow::UpdateHIDD_ui_host()
{
    if (m_b_is_hidd)
    {
        ui->btnBLEHIDDVirtualUnplug->setEnabled(m_host_valid);
        ui->btnBLEHIDConnectDisconnect->setEnabled(m_host_valid);
        ui->btnBLEHIDSendKey_1->setEnabled(m_host_valid);
        ui->btnBLEHIDSendKey_2->setEnabled(m_host_valid);
        ui->btnBLEHIDSendKey_3->setEnabled(m_host_valid);
        ui->btnBLEHIDSendKey_a->setEnabled(m_host_valid);
        ui->btnBLEHIDSendKey_b->setEnabled(m_host_valid);
        ui->btnBLEHIDSendKey_c->setEnabled(m_host_valid);
        ui->btnBLEHIDSendKey_enter->setEnabled(m_host_valid);
        ui->btnBLEHIDSendKey_esc->setEnabled(m_host_valid);
        ui->btnBLEHIDSendKey_up->setEnabled(m_host_valid);
        ui->btnBLEHIDSendKey_down->setEnabled(m_host_valid);
        ui->btnBLEHIDSendKey_left->setEnabled(m_host_valid);
        ui->btnBLEHIDSendKey_right->setEnabled(m_host_valid);
        ui->cbBLEHIDHold->setEnabled(m_host_valid);
        ui->cbBLEHIDCapLock->setEnabled(m_host_valid);
        ui->cbBLEHIDCtrl->setEnabled(m_host_valid);
        ui->cbBLEHIDAlt->setEnabled(m_host_valid);
        ui->btnBLEHIDSendReport->setEnabled(m_host_valid);

        setHIDD_buttonColor(ui->btnBLEHIDHost, m_connected ? Qt::red : Qt::white);
        if (m_host_valid)
        {
            char strBda[100];
            sprintf(strBda, "Host: %02X:%02X:%02X:%02X:%02X:%02X", m_host_ad[0],m_host_ad[1],m_host_ad[2],m_host_ad[3],m_host_ad[4],m_host_ad[5]);
            ui->btnBLEHIDHost->setText(strBda);
        }
        else
        {
            ui->btnBLEHIDHost->setText("Host: unpaired");
        }
    }
}

void MainWindow::UpdateHIDD_ui_pairing()
{
    QColor color[5] = {Qt::white, Qt::cyan, Qt::darkCyan, Qt::green, Qt::darkGreen};
    setHIDD_buttonColor(ui->btnBLEHIDPairingMode, color[m_pairing_mode < 5 ? m_pairing_mode : 0]);
    if (m_pairing_mode)
    {
        ui->btnBLEHIDPairingMode->setText("Exit Pairing Mode");
    }
    else
    {
        ui->btnBLEHIDPairingMode->setText("Enter Pairing Mode");
    }
}

// Connect to peer device
void MainWindow::on_btnBLEHIDConnectDisconnect_clicked()
{
    if (m_connected)
    {
        Log("Sending HID Disconnect Command");
        app_host_hidd_disconnect();
    }
    else
    {
        Log("Sending HID Connect Command");
        app_host_hidd_connect();
    }
}

// Send HID report
void MainWindow::on_btnBLEHIDSendReport_clicked()
{
   char szLog[80] = { 0 };
   uint8_t report[50];
   uint8_t report_len = 0;

   QVariant v1 = ui->cbBLEHIDInterupt->currentData();
   uint8_t channel = static_cast<BYTE> (v1.toUInt());

   QVariant v2 = ui->cbBLEHIDReport->currentData();
   uint8_t report_id = static_cast<BYTE>(v2.toUInt());

   QString str = ui->lineEditBLEHIDSendText->text();

   report_len = static_cast<BYTE>(GetHexValue(&(report[0]), 50, str));

   for (int i = 0; i < report_len; i++)
       sprintf(&szLog[strlen(szLog)], "%02x ", report[i]);

   Log("Sending HID Report: channel %d, report %d, %s",  channel, report_id, szLog);
   app_host_hidd_send_report(channel, report_id, report, report_len);
}

// Enter pairing mode
void MainWindow::on_btnBLEHIDPairingMode_clicked()
{
    bool cmd_enter = !m_pairing_mode;
    Log("Issue %s pairing mode command", cmd_enter ? "enter" : "exit");
    app_host_hidd_pairing_mode(cmd_enter);
}

void MainWindow::on_cbBLEHIDCapLock_clicked()
{
    if (ui->cbBLEHIDCapLock->isChecked())
       keyRpt_buf[KEYRPT_MODIFIER] |= USB_MODKEY_MASK_LEFT_SHIFT;
    else
       keyRpt_buf[KEYRPT_MODIFIER] &= ~USB_MODKEY_MASK_LEFT_SHIFT;
    btnBLEHIDSendKey();
}

void MainWindow::on_btnBLEHIDDVirtualUnplug_clicked()
{
    Log("Sending HIDD Virtual Unplug Command");
    setHIDD_HostAddr(NULL);
    app_host_hidd_virtual_unplug();
}


void MainWindow::on_cbBLEHIDCtrl_clicked()
{
    if (ui->cbBLEHIDCtrl->isChecked())
       keyRpt_buf[KEYRPT_MODIFIER] |= USB_MODKEY_MASK_LEFT_CTL;
    else
       keyRpt_buf[KEYRPT_MODIFIER] &= ~USB_MODKEY_MASK_LEFT_CTL;
    btnBLEHIDSendKey();
}

void MainWindow::on_cbBLEHIDAlt_clicked()
{
    if (ui->cbBLEHIDAlt->isChecked())
       keyRpt_buf[KEYRPT_MODIFIER] |= USB_MODKEY_MASK_LEFT_ALT;
    else
       keyRpt_buf[KEYRPT_MODIFIER] &= ~USB_MODKEY_MASK_LEFT_ALT;
    btnBLEHIDSendKey();
}

// Handle WICED HCI events for BLE/BR HID device
void MainWindow::onHandleWicedEventBLEHIDD(unsigned int opcode, unsigned char *p_data, unsigned int len)
{
    char   trace[1024];

    switch (opcode)
    {
        case HCI_CONTROL_EVENT_DEVICE_STARTED:
//            ui->btnBLEHIDPairingMode->setText("Enter Pairing Mode");
            break;

        case HCI_CONTROL_HIDD_EVENT_OPENED:
            setHIDD_HostAddr(p_data);
            if (p_data)
                Log("HID connection opened with %02X:%02X:%02X:%02X:%02X:%02X",p_data[5],p_data[4],p_data[3],p_data[2],p_data[1],p_data[0]);
            else
                Log("HID connection opened");
            break;

        case HCI_CONTROL_LE_EVENT_ADVERTISEMENT_STATE:
            Log("Advertisement state:%d", p_data[0]);
            m_pairing_mode = p_data[0];
            UpdateHIDD_ui_pairing();
            break;

        case HCI_CONTROL_HIDD_EVENT_VIRTUAL_CABLE_UNPLUGGED:
            Log("HID Virtual Cable Unplugged");
            break;

        case HCI_CONTROL_HIDD_EVENT_DATA:
            sprintf(trace, "Recv HID Report type:%d ", p_data[0]);
            for (uint i = 0; i < len - 1; i++)
                sprintf(&trace[strlen(trace)], "%02x ", p_data[i + 1]);
            Log(trace);
            break;

        case HCI_CONTROL_HIDD_EVENT_CLOSED:
            Log("HID Connection down reason: %d ", p_data[0]);
            break;
    }
}


void MainWindow::on_btnHelpHIDD_clicked()
{
    onClear();
    Log("HID Device help topic:");
    Log("");

    Log("WICED Platforms : 20706-A2, 20719-B1, 20721, 20735-B1, 208xx-A1");
    Log("Apps : use app under demo/hid on 20735-B1. Use hci_hid_host for BR-EDR or ");
    Log("       'hci_ble_hid_host' for BLE HOGP on 20706-A2 and 20719-B1, 20721");
    Log("Peer device : Windows PC or any HID host");
    Log("");

    Log("Note: For 20735-B1 and 208xx-A1 apps, see flag TESTING_USING_HCI in the app makefile");
    Log("");

    Log("- Enter/Exit Pairing Mode");
    Log("  Sets the local device to pair-able mode or exit pair-able mode");
    Log("- Connect/Disconnect");
    Log("  Connect/Disconnect with a HID host");
    Log("- Send");
    Log("  Sends the specified HID report");
    Log("  Shift Lock, etc.");
    Log("- Send Report");
    Log("  Send report for Interrupt or Control channel.");
    Log("- Debug");
    Log("  Select device firmware debug output routing. Use WICED to route to Btspy and PUART to route to BR:115200 COM port.");
    Log("- Key buttons");
    Log("  Send key HID report");
    ScrollToTop();

}