// Lab1_2_ClientServer.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include "pch.h"
#include "framework.h"
#include "Lab1_2_ClientServer.h"
#include "Message.h"
#include "Session.h"
#include <vector>
#include <cstdio>
#include <ctime>
#include <chrono>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Единственный объект приложения

CWinApp theApp;

int gMaxID = 100;
map<int, shared_ptr<Session>> gSessions;

vector<int> freeIDs;

template <typename T>
void removeInVector(std::vector<T>& vec, size_t pos)
{
    vec.erase(vec.begin() + pos);
    vector<int>(vec).swap(vec);

}

void TimeOut() {

    while (true)
    {
        for (int i = 0; i <= gMaxID; i++)
        {
            
            if (gSessions.find(i) != gSessions.end())
            {
                double  workTime = clock() - gSessions[i]->getTime();
                if (workTime > 10000)
                {
                    cout << "Client " << i << " has been disconnected due to long inactivity" << endl;
                    gSessions[i]->setConnect(false);
                    gSessions.erase(i);
                    freeIDs.push_back(i);

                }
            }
        }
        Sleep(1000);

        
    }
}

void ProcessClient(SOCKET hSock) {
    CSocket s;
    s.Attach(hSock);
    Message m;
    
        switch (m.Receive(s)) {
        case M_INIT: {
            int clientID;
            if (freeIDs.size() != 0) {
                clientID = freeIDs[0];
                removeInVector(freeIDs, 0);
            }
            else {
                gMaxID++;
                clientID = gMaxID;
            }
            auto pSession = make_shared<Session>(clientID, m.getM_Data(), clock());
            cout << "Client " << clientID << " connected" << endl;
            gSessions[pSession->getM_ID()] = pSession;

            Message::SendMessage(s, pSession->getM_ID(), A_BROCKER, M_CONFIRM);
            break;
        }

        case M_EXIT0: {
            cout << "Client " << m.getM_Header().m_From << " disconnected" << endl;
            gSessions.erase(m.getM_Header().m_From);
            Message::SendMessage(s, m.getM_Header().m_From, A_BROCKER, M_CONFIRM);
            freeIDs.push_back(m.getM_Header().m_From);
            break;
        }
        case M_GETDATA: {
            if (gSessions.find(m.getM_Header().m_From) != gSessions.end()) {

                gSessions[m.getM_Header().m_From]->setTime(clock());
                gSessions[m.getM_Header().m_From]->Send(s);
                
            }
            break;
        }
        default: {
            if (gSessions.find(m.getM_Header().m_From) != gSessions.end())
            {
                if (gSessions.find(m.getM_Header().m_To) != gSessions.end())
                {
                    gSessions[m.getM_Header().m_To]->Add(m);
                }
                else if (m.getM_Header().m_To == A_ALL)
                {
                    
                    for (const auto& i : gSessions) {
                        if (i.first != m.getM_Header().m_From) {
                            i.second->Add(m);
                        }
                    }

                   /* for (auto it = gSessions.begin(); it != gSessions.end(); ++it) {
                        if (it->first != m.getM_Header().m_From) {
                            it->second->Add(m);
                        }
                    }*/
                    /*for (int id = A_ALL+1; id < gSessions.size()+A_ALL+1; id++)
                    {
                        if (id != m.getM_Header().m_From) {
                            gSessions[id]->Add(m);
                        }
                    }*/
                }
                Message::SendMessage(s, m.getM_Header().m_From, A_BROCKER, M_CONFIRM);
                gSessions[m.getM_Header().m_From]->setTime(clock());
            }
            break;
        }
        }
    

}


void start() {
    AfxSocketInit();
    CSocket Server;
    Server.Create(11111);

    thread tt(TimeOut);
    tt.detach();

    while (true)
    {
        Server.Listen();
        CSocket s;
        Server.Accept(s);
        thread t(ProcessClient, s.Detach());
        t.detach();
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
            start();
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
