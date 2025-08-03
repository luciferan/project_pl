#define _WIN32_WINNT 0x0A00
#define ASIO_STANDALONE
//#include <asio.hpp>
#include "../_external/asio/include/asio.hpp"
#include <iostream>
#include <format>
#include <memory>
#include <array>
#include <thread>
#include <vector>

#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif //_WINSOCKAPI_

#pragma comment(lib, "ws2_32.lib")

using asio::ip::tcp;
using asio::ip::udp;
using namespace std;

class session : public std::enable_shared_from_this<session>
{
private:
    tcp::socket socket_;
    char data_[1024]{};

public:
    session(tcp::socket socket) : socket_(std::move(socket)) {}
    void start() { do_read(); }

private:
    void do_read()
    {
        auto self(shared_from_this());
        socket_.async_read_some(asio::buffer(data_), [this, self](std::error_code ec, std::size_t length) {
            if (!ec) {
                cout << "do_read" << endl;
                do_write(length);
            }
        });
    }

    void do_write(std::size_t length)
    {
        auto self(shared_from_this());
        asio::async_write(socket_, asio::buffer(data_, length), [this, self](std::error_code ec, std::size_t) {
            if (!ec) {
                cout << "do_write" << endl;
                do_read();
            }
        });
    }
};

class server
{
private:
    tcp::acceptor acceptor_;

public:
    server(asio::io_context& io_context, short port) : acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
    {
        do_accept();
    }

private:
    void do_accept()
    {
        acceptor_.async_accept([this](std::error_code ec, tcp::socket socket) {
            if (!ec) {
                std::make_shared<session>(std::move(socket))->start();
                do_accept();
            }
        });
    }
};

class tcp_connection : public std::enable_shared_from_this<tcp_connection>
{
private:
    tcp::socket _socket;
    std::string _message;
    int _seq = 0;

private:
    tcp_connection(asio::io_context& io_context) : _socket(io_context) {}

    void handle_write()
    {
        cout << format(" tcp async_send complete. seq {}", _seq) << endl;
    }

public:
    using pointer = std::shared_ptr<tcp_connection>;
    static pointer create(asio::io_context& io_context)
    {
        return pointer(new tcp_connection(io_context));
    }

    tcp::socket& socket()
    {
        return _socket;
    }

    void start()
    {
        cout << format(" tcp async_send. seq {}", _seq) << endl;

        _message = format("tcp message send {}", ++_seq);
        asio::async_write(_socket, asio::buffer(_message), std::bind(&tcp_connection::handle_write, shared_from_this()));
    }
};

class tcp_server
{
private:
    asio::io_context& _io_context;
    tcp::acceptor _acceptor;

private:
    void start_accept()
    {
        tcp_connection::pointer new_connection = tcp_connection::create(_io_context);

        cout << "tcp async_accept start" << endl;
        _acceptor.async_accept(new_connection->socket(), std::bind(&tcp_server::handle_accept, this, new_connection, asio::placeholders::error));
    }

    void handle_accept(tcp_connection::pointer new_connection, const std::error_code& error)
    {
        if (!error) {
            new_connection->start();
        }

        start_accept();
    }

public:
    tcp_server(asio::io_context& io_context, short port) : _io_context(io_context), _acceptor(_io_context, tcp::endpoint(tcp::v4(), port))
    {
        start_accept();
    }
};

class udp_server
{
private:
    udp::socket _socket;
    udp::endpoint _remote_endpoint;
    std::array<char, 1> _recv_buffer;

private:
    void start_receive()
    {
        cout << format(" udp async_receive_from") << endl;
        _socket.async_receive_from(asio::buffer(_recv_buffer), _remote_endpoint, std::bind(&udp_server::handle_receive, this, asio::placeholders::error));
    }
    void handle_receive(const std::error_code& error)
    {
        if (!error) {
            std::shared_ptr<std::string> message(new std::string("hello udp"));

            cout << format(" udp async_send_to") << endl;
            _socket.async_send_to(asio::buffer(*message), _remote_endpoint, std::bind(&udp_server::handle_send, this, message));
            start_receive();
        }
    }
    void handle_send(std::shared_ptr<std::string>)
    {
        cout << format("udp async_send_to complete") << endl;
    }

public:
    udp_server(asio::io_context& io_context, short port) : _socket(io_context, udp::endpoint(udp::v4(), port))
    {
        start_receive();
    }
};

int main(int argc, char* argv[])
{
    try {
        asio::io_context io_context;
        asio::executor_work_guard<asio::io_context::executor_type> guard(io_context.get_executor());

        tcp_server s_tcp(io_context, 16101);
        udp_server s_udp(io_context, 16201);

        vector<thread> threads;
        for (int idx = 0; idx < thread::hardware_concurrency(); ++idx) {
            threads.emplace_back([&io_context]() {
                io_context.run();
            });
        }

        io_context.run();

        for (thread& t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }
    }
    catch (std::exception e) {
        std::cerr << "exception: " << e.what() << std::endl;
    }

    return 0;
}