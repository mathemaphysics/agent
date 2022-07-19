#pragma once

#include <string>
#include <map>

#include <amqpcpp.h>

namespace agent
{
    /**
     * @brief Exchange type enum
     * 
     * Mapping the names of each enum value to its \c AMQP::ExchangeType value
     * for use in configuration via \c client.json file; see example in \c
     * config directory in the source.
     */
    static std::map<std::string, AMQP::ExchangeType> exchangeTypeMap =
    {
        {"consistent_hash", AMQP::ExchangeType::consistent_hash},
        {"direct", AMQP::ExchangeType::direct},
        {"fanout", AMQP::ExchangeType::fanout},
        {"headers", AMQP::ExchangeType::headers},
        {"topic", AMQP::ExchangeType::topic}
    };

    /**
     * @brief All bit flags
     * 
     * Equivalent mappings from name to enumeration value but for the all bit
     * flags, which are used to configure a number of things.
     */
    static std::map<std::string, const int> allBitFlags =
    {
        {"durable", AMQP::durable},
        {"autodelete", AMQP::autodelete},
        {"active", AMQP::active},
        {"passive", AMQP::passive},
        {"ifunused", AMQP::ifunused},
        {"ifempty", AMQP::ifempty},
        {"global", AMQP::global},
        {"nolocal", AMQP::nolocal},
        {"noack", AMQP::noack},
        {"exclusive", AMQP::exclusive},
        {"nowait", AMQP::nowait},
        {"mandatory", AMQP::mandatory},
        {"immediate", AMQP::immediate},
        {"redelivered", AMQP::redelivered},
        {"multiple", AMQP::multiple},
        {"requeue", AMQP::requeue},
        {"internal", AMQP::internal}
    };

    /**
     * @brief Event loop flags
     * 
     * Flags used to specify settings for event loops.
     */
    static std::map<std::string, const int> eventLoopFlags =
    {
        {"readable", AMQP::readable},
        {"writable", AMQP::writable}
    };
}
