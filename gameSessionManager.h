//
// Created by Bahadır Yurtkulu on 25/02/2023.
//

#pragma once

#include <iostream>
#include <string>
#include "build/gameSession.pb.h"
#include "build/gamePlay.pb.h"
#include "gameSession.h"
#include "util.h"

namespace rps {
    class GameSessionManager {
        using GameSessionID = uint64_t;
        using PlayerID = uint64_t;
    public:
        GameSessionManager() = default;

        std::string handle(std::string&& message) {
            protobuf_session::Request request;
            request.ParseFromString(message);

            switch (request.request_case()) {
                case protobuf_session::Request::kCreateGameSessionRequest:
                    return create_game_session(request.create_game_session_request());
                case protobuf_session::Request::kGetGameSessionDataRequest:
                    std::cout << request.get_game_session_data_request().DebugString() << std::endl;
                    return get_game_session_data(request.get_game_session_data_request());
                case protobuf_session::Request::kJoinGameSessionRequest:
                    std::cout << request.join_game_session_request().DebugString() << std::endl;
                    return join_game_session(request.join_game_session_request());
                case protobuf_session::Request::kLeaveGameSessionRequest:
                    std::cout << request.leave_game_session_request().DebugString() << std::endl;
                    return leave_game_session(request.leave_game_session_request());
                case protobuf_session::Request::REQUEST_NOT_SET:
                    static const std::string error_message = "Request not set";
                    std::cout << error_message << std::endl;
                    return error_message;
            }

            return "Hello from server"; // todo what to return here?
        }

    private:
        std::string create_game_session(const protobuf_session::CreateGameSessionRequest &request) {
            GameSession::GameSessionData data;
            data.set_id(game_session_counter_.next());
            data.set_title(request.title());
            data.set_mode(request.mode());
            data.set_number_of_rounds(request.number_of_rounds());
            data.set_state(protobuf_session::GameSessionState::WAITING_FOR_PLAYERS);

            auto serialized_response = response_to_create_game_session(data);
            game_sessions_.insert({game_session_counter_.current(), std::make_unique<GameSession>(std::move(data))});
            return serialized_response;
        }

        std::string response_to_create_game_session(const GameSession::GameSessionData& data) {
            std::cout << "*** create game session data ***\n" << data.DebugString() << std::endl;
            protobuf_session::Response response_wrapper;
            response_wrapper.mutable_create_game_session_response()->mutable_game_session()->CopyFrom(data);
            auto serialized = response_wrapper.SerializeAsString();
            return serialized;
        }

        std::string get_game_session_data(const protobuf_session::GetGameSessionDataRequest &request) {
            return "Hello from server";
        }

        std::string join_game_session(const protobuf_session::JoinGameSessionRequest& request) {

            return "Hello from server";
        }

        std::string leave_game_session(const protobuf_session::LeaveGameSessionRequest &request) {
            return "Hello from server";
        }

    private:
        std::unordered_map<GameSessionID, std::unique_ptr<GameSession>> game_sessions_;
        std::unordered_map<PlayerID, GameSessionID> player_to_game_session_;
        util::Counter game_session_counter_;
        util::Counter player_counter_;
    };
} // namespace rps