#pragma once

class Session {
private:
	int m_id;
	string m_name;
	queue<Message> m_messages;
	CRITICAL_SECTION m_CS;
	bool connect;
	clock_t  time;
public:

	void setConnect(bool c) {
		connect = c;
	}

	bool getConnect() {
		return connect;
	}

	clock_t getTime() {
		return time;
	}

	void setTime(clock_t t) {
		time = t;
	}

	int getM_ID() {
		return m_id;
	}

	void setM_ID(int id) {
		m_id = id;
	}

	string getM_Name() {
		return m_name;
	}

	void setM_Name(string n) {
		m_name = n;
	}

	Session(int ID, string Name, clock_t t)
		:m_id(ID), m_name(Name), time(t), connect(true)
	{
		InitializeCriticalSection(&m_CS);
	}
	~Session()
	{
		DeleteCriticalSection(&m_CS);
	}

	void Add(Message& msg)
	{
		EnterCriticalSection(&m_CS);
		m_messages.push(msg);
		LeaveCriticalSection(&m_CS);
	}
	void Send(CSocket& s, clock_t t)
	{
		EnterCriticalSection(&m_CS);
		if (m_messages.empty())
		{
			Message::SendMessage(s, m_id, A_BROCKER, M_NODATA);
		}
		else
		{
			m_messages.front().Send(s);
			m_messages.pop();
			time = t;
		}
		LeaveCriticalSection(&m_CS);
	}


};