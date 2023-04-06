///
#include <iostream>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include "src/websocketListener.h"
#include "src/gameSessionManager.h"

int main() {
    auto const address = boost::asio::ip::make_address("127.0.0.1");
    auto const port = static_cast<int16_t>(8080);
    auto const threads = 16; // std::max<int64_t>(1, std::thread::hardware_concurrency());
    std::cout << "Starting server on " << address << ":" << port << " with "
              << threads << " threads" << std::endl;
    try {
        auto game_session_manager = std::make_shared<rps::GameSessionManager>();

        boost::asio::thread_pool ioc(threads);
        auto listener = std::make_shared<Websocket::Listener>(
                ioc.get_executor(),
                boost::asio::ip::tcp::endpoint{address, port},
                game_session_manager);
        std::cout << "main thread id: " << std::this_thread::get_id() << std::endl;
        listener->start();
        ioc.join();
    } catch (boost::beast::system_error const& sys_err) {
        std::cerr << "Error: " << sys_err.what() << std::endl;
    }

    return 0;
}
