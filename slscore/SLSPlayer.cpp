
/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2019-2020 Edward.Wu
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include <errno.h>
#include <string.h>
#include <algorithm>
#include <map>
#include <mutex>


#include "SLSPlayer.hpp"
#include "SLSLog.hpp"

namespace {
std::mutex g_consumers_mutex;
std::map<std::string, std::vector<CSLSPlayer *>> g_active_consumers;
}

/**
 * CSLSPlayer class implementation
 */

CSLSPlayer::CSLSPlayer()
{
    m_is_write = 1;

    sprintf(m_role_name, "player");
}

CSLSPlayer::~CSLSPlayer()
{
}



int CSLSPlayer::handler()
{
    return handler_write_data() ;
}

int CSLSPlayer::uninit()
{
    unregister_active(this);
    return CSLSRole::uninit();
}

void CSLSPlayer::register_active(CSLSPlayer *player, const std::string &player_key)
{
    if (player == nullptr || player_key.empty()) {
        return;
    }

    std::lock_guard<std::mutex> lock(g_consumers_mutex);
    auto &consumers = g_active_consumers[player_key];
    if (std::find(consumers.begin(), consumers.end(), player) == consumers.end()) {
        consumers.push_back(player);
    }
    player->m_registered_player_key = player_key;
}

void CSLSPlayer::unregister_active(CSLSPlayer *player)
{
    if (player == nullptr) {
        return;
    }

    std::lock_guard<std::mutex> lock(g_consumers_mutex);
    std::string key = player->m_registered_player_key;
    if (!key.empty()) {
        auto it = g_active_consumers.find(key);
        if (it != g_active_consumers.end()) {
            auto &vec = it->second;
            vec.erase(std::remove(vec.begin(), vec.end(), player), vec.end());
            if (vec.empty()) {
                g_active_consumers.erase(it);
            }
        }
    } else {
        for (auto it = g_active_consumers.begin(); it != g_active_consumers.end();) {
            auto &vec = it->second;
            vec.erase(std::remove(vec.begin(), vec.end(), player), vec.end());
            if (vec.empty()) {
                it = g_active_consumers.erase(it);
            } else {
                ++it;
            }
        }
    }
    player->m_registered_player_key.clear();
}

std::vector<CSLSPlayer::ConsumerSnapshot> CSLSPlayer::get_active_consumers(const std::string &player_key)
{
    std::vector<ConsumerSnapshot> snapshots;

    std::lock_guard<std::mutex> lock(g_consumers_mutex);
    auto it = g_active_consumers.find(player_key);
    if (it == g_active_consumers.end()) {
        return snapshots;
    }

    for (auto *player : it->second) {
        if (player == nullptr || player->get_sock_state() != SRTS_CONNECTED) {
            continue;
        }

        ConsumerSnapshot s;
        s.connection_id = std::to_string(player->get_fd());
        char peer_name[IP_MAX_LEN] = {0};
        int peer_port = 0;
        if (player->get_peer_info(peer_name, peer_port) == SLS_OK && strlen(peer_name) > 0) {
            s.endpoint = std::string(peer_name) + ":" + std::to_string(peer_port);
        } else {
            s.endpoint = "unknown";
        }

        SRT_TRACEBSTATS stats;
        memset(&stats, 0, sizeof(stats));
        player->get_statistics(&stats, 0);
        s.bitrate = player->get_bitrate();
        s.rtt = stats.msRTT;
        s.latency = player->get_latency();
        s.buffer = stats.msRcvBuf;
        s.dropped_pkts = stats.pktRcvDrop;
        s.uptime = player->get_uptime();
        s.state = "connected";
        snapshots.push_back(s);
    }

    return snapshots;
}

