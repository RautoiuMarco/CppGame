#pragma once

#include <string>
#include <vector>
#include <crow.h>
#include <sqlite_orm/sqlite_orm.h>
import Player;

using boardElements::Player;
namespace sql = sqlite_orm;

namespace http {

    inline auto createStorage(const std::string& filename) {
        return sql::make_storage(
            filename,
            sql::make_table(
                "Players",
                sql::make_column("playerId", &Player::m_id, sql::primary_key().autoincrement()),
                sql::make_column("password", &Player::m_password),
                sql::make_column("highScore", &Player::m_highScore),
                sql::make_column("username", &Player::m_name)

            )
        );
    }
    using Storage = decltype(createStorage(""));
}
