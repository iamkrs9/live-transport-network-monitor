#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <iostream>
#include <boost/beast.hpp>

using tcp = boost::asio::ip::tcp;
namespace websocket = boost::beast::websocket;

void Log(boost::system::error_code ec) {
	std::cerr << (ec ? "Error" : "OK") << (ec ? ec.message() : "") << std::endl;
}

int main() {

	boost::asio::io_context ioc{};

	tcp::socket socket{ioc};

	boost::system::error_code ec{};
	tcp::resolver resolver{ioc};
	auto resolverIt{resolver.resolve("ltnm.learncppthroughprojects.com", "80", ec)};

	if(ec) {
		Log(ec);
		return -1;
	}
	socket.connect(*resolverIt, ec);
	if(ec) {
		Log(ec);
		return -2;
	}
	Log(ec);
	websocket::stream<boost::beast::tcp_stream> ws(std::move(socket));
	ws.handshake("ltnm.learncppthroughprojects.com", "/echo", ec);
	Log(ec);
	boost::asio::const_buffer b("ssup", sizeof("ssup"));
	ws.write(b, ec);
	Log(ec);
	boost::beast::flat_buffer b1{};
	ws.read(b1, ec);
	std::cout << boost::beast::make_printable(b1.data()) << '\n';
	return 0;
}