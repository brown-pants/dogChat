#ifndef __MESSAGE_H__
#define __MESSAGE_H__
#include <stdint.h>

class Message
{
public:
    enum { max_data_len = 4294967296, head_len = 4 };

    Message();
    char *data()                { return m_data; }
    int curLen()                { return m_curLen; }
    void setLen(int len)        { m_curLen = len; }
    uint32_t bodyLen()          { return m_bodyLen; }
    void updateHead();

private:
    char m_data[max_data_len];
    int m_curLen;
    uint32_t m_bodyLen;
};

#endif