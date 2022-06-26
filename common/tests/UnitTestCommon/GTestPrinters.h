#pragma once

#include <ostream>
#include <boost/asio/ip/udp.hpp>

struct VS_GatewayAudioMode;
struct VS_GatewayVideoMode;
struct VS_MediaChannelInfo;
template <class, class>
class basic_string_view;

void PrintTo(const VS_GatewayAudioMode& x, std::ostream* s);
void PrintTo(const VS_GatewayVideoMode& x, std::ostream* s);
void PrintTo(const boost::asio::ip::udp::endpoint& x, std::ostream*s);
void PrintTo(const VS_MediaChannelInfo& x, std::ostream* s);
void PrintTo(const basic_string_view<char, std::char_traits<char>>& x, std::ostream* s);
