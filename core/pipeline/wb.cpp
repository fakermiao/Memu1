/*
 * @Author: 苗金标
 * @Date: 2023-04-07 15:14:09
 * @LastEditors: 苗金标
 * @LastEditTime: 2023-05-23 16:35:40
 * @Description: 
 */
#include "wb.h"
namespace core{
    wb_feedback_pack wb::evaluate(){
        wb_feedback_pack feed_pack;
        feed_pack.stepOne   = false;
        feed_pack.enable    = false;
        feed_pack.flush     = false;
        feed_pack.next_pc   = 0;
        feed_pack.rd_enable = false;
        feed_pack.rd_id     = 0;
        feed_pack.rd_value  = 0;
        // printf("WB\n");
        // printf("WB\tPRIV\tip:%lx,ie:%lx,mstatus:%lx\n",priv.ip,priv.ie,priv.status);
        if(!execute_wb_fifo->is_empty()){
            instStr instInfo;
            execute_wb_fifo->pop(&instInfo);
            // printf("WB\tpc:%lx,inst:%x,valid:%d,rd_valid:%d,rd_id:%d,rd:%lx,mtvec:%lx,stvec:%lx,stval:%lx,has_exp:%d,exp_id:%d\n",instInfo.pc,instInfo.inst,instInfo.valid,instInfo.rd_valid,
            //     instInfo.rd_id,instInfo.rd_value,priv.get_mtvec(),priv.stvec,priv.stval,instInfo.has_execp,instInfo.execp_id);
            if(instInfo.enable){
                //for difftest
                // if(cpu.cycle > 96223980){
                //     printf("WB\tpc:%lx,inst:%x,debug_pc:%lx,\n",instInfo.pc,instInfo.inst,instInfo.debug_pc);
                // }
                // printf("WB\tpc:%lx,inst:%x,debug_pc:%lx\n",instInfo.pc,instInfo.inst,instInfo.debug_pc);
                feed_pack.stepOne = true;
                feed_pack.inter = false;
                cpu.pc            = instInfo.pc;
                
                rv_exc_code int_code = priv.check_and_raise_int();
                if(int_code != exc_custom_ok && (!priv.need_trap())){
                    uint64_t cause_code = static_cast<uint64_t>(int_code);
                    priv.raise_trap(instInfo.pc,csr_cause_def(cause_code,1));
                    feed_pack.enable = true;
                    feed_pack.flush  = true;
                    feed_pack.next_pc = priv.get_trap_pc();
                    feed_pack.inter = true;
                    
                    priv.post_exc();
                }else{
                    if(instInfo.valid){
                        if(instInfo.rd_valid){
                            feed_pack.rd_enable = instInfo.rd_enable;
                            feed_pack.rd_id     = instInfo.rd_id;
                            feed_pack.rd_value  = instInfo.rd_value;
                        }
                        if(instInfo.has_execp){
                            if(instInfo.execp_id == exc_breakpoint){
                                csr_cause_def cause(exc_breakpoint);
                                priv.raise_trap(instInfo.pc,cause);                                
                            }else if(instInfo.execp_id == exc_ecall_from_u){
                                if(rv_test && cpu.gpr[17] == 93){
                                    if(cpu.gpr[10] == 0){
                                        printf("***Test Pass***\n");
                                        exit(0);
                                    }
                                    else{
                                        printf("---Test Failed with value 0x%lx\n",cpu.gpr[10]);
                                        exit(1);
                                    }
                                }else{
                                    priv.ecall(instInfo.pc);
                                }
                            }else if(instInfo.execp_id == exc_instr_misalign){
                                csr_cause_def cause(instInfo.execp_id);
                                priv.raise_trap(instInfo.pc,cause,instInfo.execp_value);
                            }else if(instInfo.execp_id == exc_illegal_instr){
                                csr_cause_def cause(instInfo.execp_id);
                                priv.raise_trap(instInfo.pc,cause,instInfo.inst);
                            }
                            else{
                                csr_cause_def cause(instInfo.execp_id);
                                priv.raise_trap(instInfo.pc,cause,instInfo.rs1_value + instInfo.imm);
                            }
                        }
                        if(priv.need_trap()){
                            feed_pack.enable = true;
                            feed_pack.flush = true;
                            // printf("WB\tnext_pc:%lx\n",priv.get_trap_pc());
                            feed_pack.next_pc = priv.get_trap_pc();
                        }
                        priv.post_exc();
                    
                    }else{
                        /*非法指令*/
                        csr_cause_def cause(instInfo.execp_id);
                        uint64_t tval = (instInfo.execp_id == exc_illegal_instr) ? (instInfo.inst) : (instInfo.pc);
                        priv.raise_trap(instInfo.pc,cause,tval);
                        feed_pack.enable = true;
                        feed_pack.flush = true;
                        feed_pack.next_pc = priv.get_trap_pc();
                        priv.post_exc();
                    }
                }
                ////////
                if(feed_pack.enable && feed_pack.flush){
                    instInfo.debug_pc = feed_pack.next_pc;
                }
                feed_pack.debug_pc = instInfo.debug_pc;
            }else{
                /*enable一开始赋值true不再处理，不会发生这种情况*/
                csr_cause_def cause(instInfo.execp_id);
                priv.raise_trap(instInfo.pc,cause,instInfo.inst);
                feed_pack.enable = true;
                feed_pack.flush = true;
                feed_pack.next_pc = priv.get_trap_pc();
                priv.post_exc();
            }
        }
        // printf("WB\tflush:%d,next_pc:%lx\n",feed_pack.flush,feed_pack.next_pc);
        return feed_pack;
    }
}