//
// Created by Bahadır Yurtkulu on 25/02/2023.
//

#pragma once

#include <iostream>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <iomanip>
#include <deque>
#include "gameSessionManager.h"

namespace Websocket {
    class Session : public std::enable_shared_from_this<Session> {
    public:
        Session(boost::asio::ip::tcp::socket &&socket, std::shared_ptr<rps::GameSessionManager> game_session_manager)
                : websocket_(std::move(socket))
                , game_session_manager_(std::move(game_session_manager)) {
            websocket_.binary(true);
            client_ = websocket_.next_layer().socket().remote_endpoint().address().to_string() + ":" + std::to_string(websocket_.next_layer().socket().remote_endpoint().port());
        }

        void start() {
            boost::asio::dispatch(
                    websocket_.get_executor(),
                    boost::beast::bind_front_handler(&Session::on_start, shared_from_this())
            );
        }

    private:
        void on_start() {
            websocket_.set_option(
                    boost::beast::websocket::stream_base::timeout::suggested(boost::beast::role_type::server)
            );

            websocket_.set_option(boost::beast::websocket::stream_base::decorator(
                    [](boost::beast::websocket::response_type &res) {
                        res.set(boost::beast::http::field::server,
                                std::string(BOOST_BEAST_VERSION_STRING) + " rps game server");
                    }
            ));

            websocket_.async_accept(
                    boost::beast::bind_front_handler(&Session::on_accept, shared_from_this())
            );
        }

        void on_accept(boost::beast::error_code err) {
            if (err) {
                std::cout << "Error: " << err.message() << std::endl;
                return;
            }

            game_session_manager_->new_connection(client_);
            async_read();
        }

        void async_read() {
            buffer_.clear();
            websocket_.async_read(
                    buffer_,
                    boost::beast::bind_front_handler(&Session::on_read, shared_from_this())
            );
        }

        void on_read(boost::beast::error_code err, std::size_t bytes_transferred) {
            if (err) {
                std::cout << "Error: " << err.message() << std::endl;
                return;
            }
            std::cout << "Reading from client: " << client_ << std::endl << std::flush; // TODO remove
            async_reply(game_session_manager_->handle(client_, std::move(boost::beast::buffers_to_string(buffer_.data()))));
        }

        void async_reply(std::string&& message) {
            outgoing_messages_.emplace_back(message);
            if (not outgoing_messages_.empty()) {
                async_reply_all();
                return;
            }
        }

        void async_reply_all() {
            if (outgoing_messages_.empty()) {
                async_read();
                return;
            }
            websocket_.async_write(
                    boost::asio::buffer(outgoing_messages_.front()),
                    boost::beast::bind_front_handler(&Session::on_write, shared_from_this())
            );
        }

        void on_write(boost::beast::error_code err, std::size_t /* bytes_transferred */) {
            if (err) {
                std::cout << "Error: " << err.message() << std::endl;
                return;
            }
            outgoing_messages_.pop_front();
            async_reply_all();
        }

    private:
        boost::beast::websocket::stream<boost::beast::tcp_stream> websocket_;
        boost::beast::flat_buffer buffer_;
        std::shared_ptr<rps::GameSessionManager> game_session_manager_;
        std::deque<std::string> outgoing_messages_;
        std::string client_;
    };

} // namespace Websocket
