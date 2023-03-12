//
// Created by Bahadır Yurtkulu on 26/02/2023.
//

#include <iostream>
#include <string>
#include <memory>
#include <utility>
#include "../build/gameSession.pb.h"


namespace rps {
    class GameSession {
    public:
        using GameSessionData = protobuf_session::GameSession;

    public:
        explicit GameSession(GameSessionData&& data) : data_(std::move(data)) {}

        const GameSessionData& data() const {
            return data_;
        }

        GameSessionData& mutable_data() {
            return data_;
        }

    private:
        GameSessionData data_;
    };
} // namespace rps

