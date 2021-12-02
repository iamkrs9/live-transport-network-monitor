#include "websocket-client.h"
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/system/error_code.hpp>

#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>
using NetworkMonitor::WebSocketClient;
using tcp = boost::asio::ip::tcp;
namespace websocket = boost::beast::websocket;

static void Log(const std::string &s, boost::system::error_code ec) {
    std::cerr << "[" << std::setw(20) << s << "] "
              << (ec ? "Error" : "OK")
              << (ec ? ec.message() : "")
              << '\n';
}

WebSocketClient::WebSocketClient(const std::string &url, const std::string &endpoint,
                                 const std::string &port, boost::asio::io_context &ioc) : url_{url},
                                 endpoint_{endpoint}, port_{port}, ws_{boost::asio::make_strand(ioc)},
                                 resolver_{boost::asio::make_strand(ioc)} {}

WebSocketClient::~WebSocketClient() = default;

void WebSocketClient::Connect(std::function<void (boost::system::error_code)> onConnect,
                              std::function<void (boost::system::error_code, std::string&&)> onMessage,
                              std::function<void (boost::system::error_code)> onDisconnect) {
    onConnect_ = onConnect;
    onMessage_ = onMessage;
    onDisconnect_ = onDisconnect;

    closed_ = false;
    resolver_.async_resolve(url_, port_, [this](auto ec, auto resolverIt) {
        std::cout << "Connect" << '\n';
        OnResolve(ec, resolverIt);
        }
    );
}

void WebSocketClient::OnResolve(boost::system::error_code ec, tcp::resolver::iterator resolverIt) {
    if(ec) {
        Log("OnResolve", ec);
        if(onConnect_) {
            onConnect_(ec);
        }
        return;
    }
    ws_.next_layer().expires_after(std::chrono::seconds(5));

    ws_.next_layer().async_connect(*resolverIt, [this](auto ec){
        std::cout << "OnResolve" << '\n';
        OnConnect(ec);
        }
    );
}

void WebSocketClient::OnConnect(boost::system::error_code ec) {
    if(ec) {
        Log("OnConnect", ec);
        if(onConnect_) {
            onConnect_(ec);
        }
        return;
    }
    ws_.next_layer().expires_never();
    ws_.set_option(websocket::stream_base::timeout::suggested(
            boost::beast::role_type::client
    ));
    ws_.async_handshake(url_, endpoint_, [this](auto ec){
        std::cout << "OnConnect" << '\n';
        OnHandshake(ec);
        }
    );
}

void WebSocketClient::OnHandshake(boost::system::error_code ec) {
    if(ec) {
        Log("OnHandshake", ec);
        if(onConnect_) {
            onConnect_(ec);
        }
        return;
    }
    std::cout << "OnHandshake" << '\n';
    ListenToIncomingMessage(ec);

    if(onConnect_) {
        onConnect_(ec);
    }
}

void WebSocketClient::ListenToIncomingMessage(boost::system::error_code ec) {
    if(ec == boost::asio::error::operation_aborted) {
        if(onDisconnect_ && ! closed_) {
            onDisconnect_(ec);
        }
        return;
    }

    ws_.async_read(reader_, [this](auto ec, int a){
        OnRead(ec, a);
        std::cout << "L" << '\n';
        ListenToIncomingMessage(ec);
        }
    );
}

void WebSocketClient::OnRead(boost::system::error_code ec, size_t nBytes) {
    if(ec) {
        return;
    }
    std::cout << "OnRead" << '\n';
    std::string msg{boost::beast::buffers_to_string(reader_.data())};
    reader_.consume(nBytes);
    if(onMessage_) {
        onMessage_(ec, std::move(msg));
    }
}

void WebSocketClient::Send(const std::string &message,
                           std::function<void(boost::system::error_code)> onSend) {
    ws_.async_write(boost::asio::buffer(message), [onSend](auto ec, auto){
        if(onSend) {
            onSend(ec);
            }
        std::cout << "Send" << '\n';
        }
    );
}

void WebSocketClient::Close(std::function<void (boost::system::error_code)> onClose) {
    closed_ = true;
    ws_.async_close(websocket::close_code::none, [onClose](auto ec){
        if(onClose) {
            onClose(ec);
        }
        std::cout << "Close" << '\n';
    });
}