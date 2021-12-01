#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <iostream>
#include <boost/beast.hpp>
#include <string>

using tcp = boost::asio::ip::tcp;
namespace websocket = boost::beast::websocket;

void Log(boost::system::error_code ec) {
	std::cerr << (ec ? "Error" : "OK") << (ec ? ec.message() : "") << std::endl;
}

void OnReceive(boost::beast::flat_buffer &b1, boost::system::error_code &ec) {
	if(ec) {
		Log(ec);
		return;
	}
	std::cout << boost::beast::make_printable(b1.data()) << '\n';
}

void OnSend(websocket::stream<boost::beast::tcp_stream> &ws, boost::beast::flat_buffer &b1, boost::system::error_code &ec) {
	if(ec) {
		Log(ec);
		return;
	}
	ws.async_read(b1, [&b1](auto ec, int a){
		OnReceive(b1, ec);
	});
}

void OnHandshake(websocket::stream<boost::beast::tcp_stream> &ws, boost::asio::const_buffer &b, boost::beast::flat_buffer &b1, boost::system::error_code &ec) {
	if(ec) {
		Log(ec);
		return;
	}
	ws.async_write(b, [&ws, &b1](auto ec, int a){
		OnSend(ws, b1, ec);
	});
}

void OnConnect(websocket::stream<boost::beast::tcp_stream> &ws, std::string &url, std::string &endpoint, boost::asio::const_buffer &b, boost::beast::flat_buffer &b1, boost::system::error_code &ec) {

	if(ec) {
		Log(ec);
		return;
	}
	ws.async_handshake(url, endpoint, [&ws, &b, &b1](auto ec){
		OnHandshake(ws, b, b1, ec);
	});
}


void OnResolve(websocket::stream<boost::beast::tcp_stream> &ws, std::string &url, std::string &endpoint, boost::asio::const_buffer &b, boost::beast::flat_buffer &b1, boost::system::error_code &ec, tcp::resolver::iterator resolverIt) {
	
	if (ec) {
		Log(ec);
		return;
	}
	ws.next_layer().async_connect(*resolverIt, [&ws, &url, &endpoint, &b, &b1](auto ec){
		OnConnect(ws, url, endpoint, b, b1, ec);
	});
	
}

int main() {

	std::string url{"ltnm.learncppthroughprojects.com"};
	std::string endpoint{"/echo"};
	std::string port{"80"};
	std::string message{"Hi"};

	boost::asio::io_context ioc{};
	websocket::stream<boost::beast::tcp_stream> ws{ioc};
	boost::asio::const_buffer b{message.c_str(), message.size()};
	boost::beast::flat_buffer b1{};

	tcp::resolver resolver{ioc};
	resolver.async_resolve(url, port, [&ws, &url, &endpoint, &b, &b1](auto ec, auto resolverIt){
		OnResolve(ws, url, endpoint, b, b1, ec, resolverIt);
	});

	ioc.run();

	return 0;
}