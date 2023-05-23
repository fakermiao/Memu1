/*
 * @Author: 苗金标
 * @Date: 2023-04-28 16:03:31
 * @LastEditors: 苗金标
 * @LastEditTime: 2023-05-05 16:46:49
 * @Description: 
 */
#pragma once
#include "../common.h"
#include "../config.h"
#include "fifo.h"
namespace component{
    static const uint32_t phy_reg_num_bitmap_size = (PHY_REG_NUM + bitsizeof(uint64_t) - 1) / (bitsizeof(uint64_t));

    typedef struct checkpoint_t{
        uint64_t rat_phy_map_table_valid[phy_reg_num_bitmap_size];
        uint64_t rat_phy_map_table_visible[phy_reg_num_bitmap_size];
        uint64_t phy_regfile_data_valid[phy_reg_num_bitmap_size];

        uint32_t rat_phy_map_table[PHY_REG_NUM];
        uint32_t global_history;
        uint32_t local_history;

        //clone the info into cp
        void clone(checkpoint_t &cp){
            memcpy(&cp.rat_phy_map_table_valid,&rat_phy_map_table_valid,sizeof(rat_phy_map_table_valid));
            memcpy(&cp.rat_phy_map_table_visible,&rat_phy_map_table_visible,sizeof(rat_phy_map_table_visible));
            cp.global_history = global_history;
            cp.local_history  = local_history;
        }

        virtual void print(){

        }
    }checkpoint_t;

    class checkpoint_buffer : public fifo<checkpoint_t>{
        private:
            enum class sync_request_type_t{
                pop,set_item
            };

            typedef struct sync_request_t{
                sync_request_type_t req;
                uint32_t arg1;
                checkpoint_t arg2;
            }sync_request_t;

            std::queue<sync_request_t> sync_request_q;

            bool check_new_id_valid(uint32_t id){
                if(this->is_full()){
                    return false;
                }else if(this->wstage == this->rstage){
                    return !((id>=this->rptr) && (id<this->wptr));
                }else{
                    return !(((id>=this->rptr)&&(id<this->size))||(id<this->wptr));
                }
            }

            bool check_id_valid(uint32_t id){
                if(this->is_empty()){
                    return false;
                }else if(this->wstage == this->rstage){
                    return (id>=this->rptr) && (id<wptr);
                }else{
                    return ((id>=this->rptr) &&  (id<this->size)) || (id<this->wptr);
                }
            }

        public:
            checkpoint_buffer(uint32_t size) : fifo<checkpoint_t>(size){}
            
    };
}
