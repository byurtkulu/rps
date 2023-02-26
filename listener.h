//
// Created by Bahadır Yurtkulu on 25/02/2023.
//

#pragma once

#include <iostream>
#include <list>
#include <memory>
#include <boost/asio.hpp>
#include "gameSessionManager.h"
#include "session.h"

namespace Websocket {
    class Listener : public std::enable_shared_from_this<Listener> {
    public:
        Listener(const boost::asio::any_io_executor &executor,
                 const boost::asio::ip::tcp::endpoint &endpoint,
                 std::shared_ptr<rps::GameSessionManager> game_session_manager)
                : executor_(executor)
                , acceptor_(make_strand(executor))
                , game_session_manager_(std::move(game_session_manager)) {
            acceptor_.open(endpoint.protocol());
            acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
            acceptor_.bind(endpoint);
            acceptor_.listen(boost::asio::ip::tcp::acceptor::max_listen_connections);
        }

        void start() {
            async_accept();
        }

    private:
        void async_accept() {
            acceptor_.async_accept(
                    boost::asio::make_strand(executor_),
                    boost::beast::bind_front_handler(&Listener::on_accept, shared_from_this())
            );
        }

        void on_accept(boost::beast::error_code err, boost::asio::ip::tcp::socket socket) {
            if (err) {
                std::cout << "Error: " << err.message() << std::endl;
            } else {
                auto session = std::make_shared<Session>(std::move(socket), game_session_manager_);
                sessions_.emplace_back(session);
                session->run();
            }
            async_accept();
        }

    private:
        boost::asio::any_io_executor executor_;
        boost::asio::ip::tcp::acceptor acceptor_;
        std::shared_ptr<rps::GameSessionManager> game_session_manager_;
        std::list<std::weak_ptr<Session>> sessions_;
    };
} // namespace Websocket