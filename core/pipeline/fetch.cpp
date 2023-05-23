/*
 * @Author: 苗金标
 * @Date: 2023-03-31 20:40:33
 * @LastEditors: 苗金标
 * @LastEditTime: 2023-05-23 09:43:19
 * @Description: 
 */
#include "fetch.h"
namespace core{
    void fetch::evaluate(bru_feedback_pack bru_feedback_pack_t,wb_feedback_pack wb_feedback_pack_t){
        /*先按照简单逻辑写，后面根据nutshell进行修改而且还要根据分支预测、缓存进行修改*/
        // printf("FETCH\tpc:%lx,jump:%d,feedback_enable:%d,feedback_pc:%lx,feedback_jump:%d\n",this->pc,this->jump_wait,
        // bru_feedback_pack_t.enable,bru_feedback_pack_t.next_pc,bru_feedback_pack_t.jump);
        if(!(wb_feedback_pack_t.enable && wb_feedback_pack_t.flush))
        {
            if(jump_wait){
                if(bru_feedback_pack_t.enable){
                    this->jump_wait = false;
                    if(bru_feedback_pack_t.jump){
                        this->pc = bru_feedback_pack_t.next_pc;
                    }else{
                        this->pc += 4;//待补充
                    }
                }
            }else if(!fetch_decode_fifo->is_full()){
                if(pc % 4){
                    this->jump_wait = true;
                    fetch_decode_pack send_pack;
                    memset(&send_pack,0,sizeof(send_pack));
                    instStr fetch_info;
                    fetch_info.enable = true;
                    fetch_info.pc     = this->pc;
                    fetch_info.inst   = 0;
                    fetch_info.valid  = false;
                    fetch_info.has_execp = true;
                    fetch_info.execp_id  = exc_instr_misalign;
                    fetch_info.debug_pc = this->pc + 4;

                    send_pack.fetch_decode[0] = fetch_info;
                    this->fetch_decode_fifo->push(send_pack);
                }else{
                    fetch_decode_pack send_pack;
                    memset(&send_pack,0,sizeof(send_pack));
                    uint32_t offset = (pc / 4) % FETCH_WIDTH;
                    uint32_t len    = FETCH_WIDTH - offset;
                    
                    for(uint32_t pos = 0;pos < len;pos++){
                        instStr fetch_info;
                        uint32_t inst_v = 0;
                        // mem->read(this->pc,&inst_v,4);
                        rv_exc_code exp_id = priv.va_if(this->pc,4,(uint8_t*)&inst_v);
                        if(exp_id == exc_custom_ok){
                            fetch_info.valid = true;
                            fetch_info.has_execp = false;
                        }else{
                            fetch_info.has_execp = true;
                            fetch_info.execp_id  = exp_id;
                            fetch_info.valid = false;
                        }

                        fetch_info.enable = true;
                        fetch_info.pc     = this->pc;
                        fetch_info.inst   = inst_v;
                        this->jump_wait   = ((inst_v & 0x7f) == 0x6f) || ((inst_v & 0x7f) == 0x67) || ((inst_v & 0x7f) == 0x63) || 
                                            ((inst_v & 0x7f) == 0x0f) || ((inst_v & 0x7f) == 0x73);
                        // this->jump_wait   = ((inst_v & 0x7f) == 0x6f) || ((inst_v & 0x7f) == 0x67) || ((inst_v & 0x7f) == 0x63);
                        // printf("FETCH\tpc:%lx,inst:%x,jump:%d,valid:%d,has_exp:%d\n",fetch_info.pc,fetch_info.inst,this->jump_wait,fetch_info.valid,fetch_info.has_execp);
                        // if((inst_v & 0x707f) == 0x100f){
                        //     printf("---%ld\n",cpu.cycle);
                        //     assert(0);
                        // }
                        // if((pc & 0xffff) == 0x2a5c){
                        //     printf("---%ld\n",cpu.cycle);
                        //     assert(0);
                        // }
                        // if((inst_v & 0xfe007fff) == 0x12000073){
                        //     printf("pc-------:%lx\n",pc);
                        //     assert(0);
                        // }
                        send_pack.fetch_decode[pos] = fetch_info;

                        if(this->jump_wait)
                            break;
                        else if(fetch_info.enable)
                            this->pc += 4;
                    }
                    this->fetch_decode_fifo->push(send_pack);
                }
            }
        }else{
            // printf("hello:%lx\n",wb_feedback_pack_t.next_pc);
            this->pc = wb_feedback_pack_t.next_pc;
            this->jump_wait = false;
            this->fetch_decode_fifo->flush();
        }

    }

    void fetch::print(){
        printf("pc = 0x%lx,jump_wait = %d\n",this->pc,this->jump_wait);
    }
}