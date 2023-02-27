//
// Created by Bahadır Yurtkulu on 25/02/2023.
//

#pragma once

#include <iostream>
#include <string>
#include "../build/gameSession.pb.h"
#include "../build/gamePlay.pb.h"
#include "gameSession.h"
#include "../util/util.h"

namespace rps {
    class GameSessionManager {
        using GameSessionID = int64_t;
        using PlayerID = int64_t;
    public:
        GameSessionManager() = default;

        std::string handle(std::string&& message) {
            protobuf_session::Request request;
            request.ParseFromString(message);

            switch (request.request_case()) {
                case protobuf_session::Request::kCreateGameSessionRequest:
                    return create_game_session(request.create_game_session_request());
                case protobuf_session::Request::kGetGameSessionDataRequest:
                    return get_game_session_data(request.get_game_session_data_request());
                case protobuf_session::Request::kJoinGameSessionRequest:
                    return join_game_session(request.join_game_session_request());
                case protobuf_session::Request::REQUEST_NOT_SET:
                    std::cout << "Request not set" << std::endl;
                    return "Request not set";
                default:
                    std::cout << "Unknown request" << std::endl;
                    return "Unknown request";
            }
        }

    private:
        std::string create_game_session(const protobuf_session::CreateGameSessionRequest &request) {
            GameSession::GameSessionData data;
            data.set_id(game_session_counter_.next());
            data.set_title(request.title());
            data.set_mode(request.mode());
            data.set_state(protobuf_session::GameSessionState::WAITING_FOR_PLAYERS);

            if (data.mode() == protobuf_session::GameMode::PVE) {
                protobuf_session::Player* ai = data.mutable_player1();
                ai->set_id(player_counter_.next());
                ai->set_name("bob");
                ai->set_is_ai(true);
            }

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

        std::string get_game_session_data(const protobuf_session::GetGameSessionDataRequest& request) {
            std::cout << "Get game session data request: \n" << request.DebugString() << std::endl;
            auto itr = game_sessions_.find(request.game_session_id()); //->data().SerializeAsString();
            if (itr == game_sessions_.end()) {
                std::cout << "Game session not found" << std::endl;
                return error_serialized(protobuf_session::ErrorCode::GAME_SESSION_NOT_FOUND);
            }
            return response_to_get_game_session_data(itr->second->data());
        }

        std::string response_to_get_game_session_data(const GameSession::GameSessionData& data) {
            protobuf_session::Response response_wrapper;
            response_wrapper.mutable_get_game_session_data_response()->mutable_game_session()->CopyFrom(data);
            return response_wrapper.SerializeAsString();
        }

        std::string join_game_session(const protobuf_session::JoinGameSessionRequest& request) {
            std::cout << "Join game session request: \n" << request.DebugString() << std::endl;
            auto itr = game_sessions_.find(request.game_session_id()); //->data().SerializeAsString();
            if (itr == game_sessions_.end()) {
                std::cout << "Game session not found" << std::endl;
                return error_serialized(protobuf_session::ErrorCode::GAME_SESSION_NOT_FOUND);
            }

            protobuf_session::GameSession& data = itr->second->mutable_data();

            if (data.state() != protobuf_session::GameSessionState::WAITING_FOR_PLAYERS) {
                std::cout << "Game session is not waiting for players" << std::endl;
                return error_serialized(protobuf_session::ErrorCode::GAME_SESSION_NOT_WAITING_FOR_PLAYERS);
            }

            protobuf_session::Player* player;
            if (not data.has_player1()) {
                player = data.mutable_player1();
                player->set_id(player_counter_.next());
                player->set_name(request.player_name());
                player->set_is_ai(false);
            } else if (not data.has_player2()) {
                player = data.mutable_player2();
                player->set_id(player_counter_.next());
                player->set_name(request.player_name());
                player->set_is_ai(false);
            } else {
                std::cout << "Game session is full!" << std::endl;
                return error_serialized(protobuf_session::ErrorCode::GAME_SESSION_FULL);
            }

            if (data.has_player1() and data.has_player2()) {
                data.set_state(protobuf_session::GameSessionState::PLAYING);
            }

            return response_to_join_game_session(data, player->id());
        }

        std::string response_to_join_game_session(const GameSession::GameSessionData& data, int64_t player_id) {
            protobuf_session::Response response_wrapper;
            response_wrapper.mutable_join_game_session_response()->mutable_game_session()->CopyFrom(data);
            response_wrapper.mutable_join_game_session_response()->set_player_id(player_id);
            return response_wrapper.SerializeAsString();
        }

        std::string error_serialized(protobuf_session::ErrorCode err) {
            protobuf_session::Response response_wrapper;
            response_wrapper.mutable_error()->set_error_code(err);
            return response_wrapper.SerializeAsString();
        }

    private:
        std::unordered_map<GameSessionID, std::unique_ptr<GameSession>> game_sessions_;
        std::unordered_map<PlayerID, GameSessionID> player_to_game_session_;
        util::Counter game_session_counter_;
        util::Counter player_counter_;
    };
} // namespace rps