/*
 * @Author: 苗金标
 * @Date: 2023-04-03 18:58:37
 * @LastEditors: 苗金标
 * @LastEditTime: 2023-05-23 10:23:47
 * @Description: 
 */
#pragma once
#include "../common.h"
#include "../component/fifo.h"
#include "execute/priv.h"
typedef struct wb_feedback_pack{
    bool     stepOne;
    bool     enable;
    bool     flush;
    bool     inter;
    uint64_t next_pc;
    uint64_t debug_pc;
    
    bool     rd_enable;
    uint32_t rd_id;
    int64_t  rd_value;
}wb_feedback_pack;
namespace core{
    class wb{
        public:
            component::fifo<instStr> *execute_wb_fifo;
            Priv& priv;
            bool rv_test;

            wb(component::fifo<instStr> *execute_wb_fifo,Priv& priv,bool rv_test):execute_wb_fifo(execute_wb_fifo),priv(priv),rv_test(rv_test){}

            virtual wb_feedback_pack evaluate();
    };
}

