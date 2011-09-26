/*
 *  Copyright 2011 Mikko Koppanen <mikko@kuut.io>
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.                 
 */
#ifndef PZQ_SOCKET_HPP
# define PZQ_SOCKET_HPP

#include "pzq.hpp"

namespace pzq
{
    class socket_t : public zmq::socket_t
    {
    public:
        socket_t (zmq::context_t& ctx, int io_threads) : zmq::socket_t (ctx, io_threads)
        {}

        bool send_many (pzq_mp_message &parts, int flags)
        {
            pzq_mp_message_it it;
            size_t i, elements = parts.size ();

            for (i = 0, it = parts.begin (); it != parts.end (); it++, i++)
            {
                int snd_flags = flags;
                if (i < (elements - 1))
                {
                    snd_flags = flags | ZMQ_SNDMORE;
                }
                // TODO: what happens if send fails in the middle of multi-part
                if (send ((*(*it).get ()), snd_flags) == false)
                    return false;
            }
            return true;
        }

        bool send_many (pzq_mp_message &parts)
        {
            return send_many (parts, 0);
        }

        int recv_many (pzq_mp_message &parts, int flags)
        {
            int64_t more;
            size_t moresz = sizeof (int64_t);

            int i = 0;
            do {
                boost::shared_ptr <zmq::message_t> message_part (new zmq::message_t);
                if (recv (message_part.get (), flags) == false)
                    return 0;

                parts.push_back (message_part);
                getsockopt (ZMQ_RCVMORE, &more, &moresz);
                ++i;
            } while (more);

            return i;
        }

        int recv_many (pzq_mp_message &parts)
        {
            return recv_many (parts, 0);
        }
    };
}
#endif