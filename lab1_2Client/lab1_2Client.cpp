// lab1_2Client.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include "pch.h"
#include "framework.h"
#include "lab1_2Client.h"
#include "Message.h"
#include <string>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Единственный объект приложения

CWinApp theApp;

int ClientId;
mutex hMutex;
bool connection;

void canalStart(CSocket& hS) {
    hS.Create();
    hS.Connect(_T("127.0.0.1"), 11111);
}

void canalStop(CSocket& hS) {
    hS.Close();
}

void getData() {
    while (true) {
        CSocket client;
        canalStart(client);
        Message m;
        Message::SendMessage(client, 0, ClientId, M_GETDATA);
        MsgHeader h_message;
        h_message = m.Receive(client);
        canalStop(client);
        if (h_message.m_Type == M_TEXT) {
            hMutex.lock();
            cout << "Message from client "<<m.getM_Header().m_From<<": "<<m.getM_Data() << endl;
            hMutex.unlock();
        }

        if (h_message.m_Type == M_EXIT1) {
            hMutex.lock();
            cout << "You have been disconnected due to long inactivity" << endl;
            hMutex.unlock();
            connection = false;
            cout << "Want to connect to a server again? (0/1)" << endl;
        }
            
        
    }
}

void ConnectToServer(Message& m, MsgHeader& h_message, CSocket& client) {

    AfxSocketInit();
    canalStart(client);
    Message::SendMessage(client, 0, 0, M_INIT);
    h_message = m.Receive(client);
    canalStop(client);

    if (h_message.m_Type == M_CONFIRM) {
        ClientId = h_message.m_To;
        hMutex.lock();
        cout << "Your ID is " << ClientId << endl;
        hMutex.unlock();
        thread t(getData);
        t.detach();
        connection = true;
    }
    else {
        cout << "EORROR::CLIENT NOT CONNECTED" << endl;
        return;
    }
}

void start() {
    cout << "Want to connect to a server? (0/1)" << endl;
    int answer;
    cin >> answer;
    if (answer == 1)
        connection = true;
    if (answer == 0)
        return;
    MsgHeader h_message;
    CSocket client;
    Message m;
    ConnectToServer(m, h_message, client);
        while (connection) {
            
            cout << "Press 0 for send Message \n Press 1 for Exit" << endl;
            cin >> answer;

            switch (answer)
            {
            case 0: {
                if (connection) {
                    string str;
                    cout << "Enter ID of client" << endl;
                    int ID;
                    cin >> ID;
                    cout << "Enter your Message" << endl;
                    cin.ignore();
                    getline(cin, str);
                    canalStart(client);
                    Message::SendMessage(client, ID, ClientId, M_TEXT, str);
                    h_message = m.Receive(client);
                    canalStop(client);
                    hMutex.lock();
                    if (h_message.m_Type == M_CONFIRM) {
                        cout << "SUCCES::MESSAGE WAS SEND" << endl;
                    }
                    else {
                        cout << "FAIL::MESSAGE WAS NOT SEND" << endl;
                    }
                    hMutex.unlock();
                    break;
                }
                else {
                    return;
                }
                
            }

            case 1: {
                if (connection) {
                    canalStart(client);
                    Message::SendMessage(client, 0, ClientId, M_EXIT0);
                    h_message = m.Receive(client);
                    hMutex.lock();
                    if (h_message.m_Type == M_CONFIRM)
                    {
                        cout << "SUCCES!" << endl;
                    }
                    else cout << "FAIL" << endl;
                    hMutex.unlock();
                    connection = false;
                    return;
                }
                else {
                    connection = true;
                    ConnectToServer(m, h_message, client);
                }
                break;
            }
            default:
                cout << "ERROR::PLEASE, PRESS 0 OR 1" << endl;
                break;
            }
        }
    
}

int main()
{
    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr)
    {
        // инициализировать MFC, а также печать и сообщения об ошибках про сбое
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: вставьте сюда код для приложения.
            wprintf(L"Критическая ошибка: сбой при инициализации MFC\n");
            nRetCode = 1;
        }
        else
        {
            HWND hwnd = GetConsoleWindow();
            HMENU hmenu = GetSystemMenu(hwnd, FALSE);
            EnableMenuItem(hmenu, SC_CLOSE, MF_GRAYED);
            start();
            // TODO: вставьте сюда код для приложения.
        }
    }
    else
    {
        // TODO: измените код ошибки в соответствии с потребностями
        wprintf(L"Критическая ошибка: сбой GetModuleHandle\n");
        nRetCode = 1;
    }

    return nRetCode;
}
