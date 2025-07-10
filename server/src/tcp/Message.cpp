#include "Message.h"
#include <string.h>
#include <boost/asio.hpp>

Message::Message()
    : m_curLen(0), m_bodyLen(0)
{
    
}

void Message::updateHead()
{
    memcpy(&m_bodyLen, m_data, sizeof(int32_t));
    m_bodyLen = boost::asio::detail::socket_ops::network_to_host_long(m_bodyLen);
}