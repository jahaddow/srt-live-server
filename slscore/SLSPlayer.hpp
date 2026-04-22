
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


#ifndef _SLSPlayer_INCLUDE_
#define _SLSPlayer_INCLUDE_

#include "SLSRole.hpp"
#include <string>
#include <vector>

/**
 * CSLSPlayer
 */
class CSLSPlayer: public CSLSRole
{
public :
	CSLSPlayer();
	~CSLSPlayer();

    struct ConsumerSnapshot {
        std::string connection_id;
        std::string endpoint;
        int bitrate = 0;
        int rtt = 0;
        int latency = 0;
        int buffer = 0;
        int dropped_pkts = 0;
        int uptime = 0;
        std::string state;
    };

    virtual int handler();
    virtual int uninit() override;

    static void register_active(CSLSPlayer *player, const std::string &player_key);
    static void unregister_active(CSLSPlayer *player);
    static std::vector<ConsumerSnapshot> get_active_consumers(const std::string &player_key);

private:
    std::string m_registered_player_key;


};


#endif
