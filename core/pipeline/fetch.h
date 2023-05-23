/*
 * @Author: 苗金标
 * @Date: 2023-03-31 20:40:22
 * @LastEditors: 苗金标
 * @LastEditTime: 2023-04-23 10:03:14
 * @Description: 
 */
#pragma once
#include <queue>
#include "../common.h"
#include "../component/memory.h"
#include "../component/fifo.h"
#include "fetch_decode.h"
#include "execute/bru.h"
#include "wb.h"
#include "execute/priv.h"

namespace core{
    class fetch{
        public:
            uint64_t pc;
            uint64_t npc;
            Priv& priv;
            component::memory*  mem;
            bool     jump_wait;
            component::fifo<fetch_decode_pack> *fetch_decode_fifo;

            fetch(uint64_t npc,Priv& priv,component::memory* mem,component::fifo<fetch_decode_pack> *fetch_decode_fifo):
                npc(npc),priv(priv),mem(mem),fetch_decode_fifo(fetch_decode_fifo){
                    this->pc = npc;
                    this->jump_wait = false;

            }
            void evaluate(bru_feedback_pack bru_feedback_pack_t,wb_feedback_pack wb_feedback_pack_t);
            void print();
    };
}
