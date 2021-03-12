#include<iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sstream>
using namespace std;
void printreg(int[]);
void printmem(string[]);
void initialize_reg(int[]);
void initialize_mem(string[]);
void initialize_delays(int[]);
int reg_stoi(string);
int main(){
     ifstream file,file1;
     file.open("input.txt");
     file1.open("delays.txt");
     string line;
     int delays[15];
     int totalcycle=0;
     int instcount=0;
     initialize_delays(delays);
     //Initialising delay array to zeroes
     //-------------------------------
     while (file1>>line){
          if(line.compare("add")==0){file1>>line; delays[0]=stoi(line);}
          else if(line.compare("sub")==0){file1>>line;delays[1]=stoi(line);}
          else if(line.compare("sll")==0){file1>>line;delays[2]=stoi(line);}
          else if(line.compare("srl")==0){file1>>line;delays[3]=stoi(line);}
          else if(line.compare("sw")==0){file1>>line;delays[4]=stoi(line);}
          else if(line.compare("lw")==0){file1>>line;delays[5]=stoi(line);}
          else if(line.compare("j")==0){file1>>line;delays[6]=stoi(line);}
          else if(line.compare("jal")==0){file1>>line;delays[7]=stoi(line);}
          else if(line.compare("jr")==0){file1>>line;delays[8]=stoi(line);}
          else if(line.compare("beq")==0){file1>>line;delays[9]=stoi(line);}
          else if(line.compare("bne")==0){file1>>line;delays[10]=stoi(line);}
          else if(line.compare("bgt")==0){file1>>line;delays[11]=stoi(line);}
          else if(line.compare("bge")==0){file1>>line;delays[12]=stoi(line);}
          else if(line.compare("blt")==0){file1>>line;delays[13]=stoi(line);}
          else if(line.compare("ble")==0){file1>>line;delays[14]=stoi(line);}
     }
     //-------------------------------
     int reg[32];
     string mem[4096];
     //Initialising Register array and Memory file with zeroes
     //-------------------------------
     initialize_reg(reg);
     initialize_mem(mem);
     //-------------------------------
     int pc=0;
     //Loading Program into memory
     //-------------------------------
     getline(file, line);
     while(file){
          cout << pc << " = " << line << endl;
          mem[pc++]=line;
          getline(file, line);
     }
     //------------------------------
     //TEST CASE FOR FIBONACCI SERIES
     reg[1]=1;
     reg[2]=1;
     reg[3]=10;
     reg[4]=0;
     reg[5]=1;
     reg[29]=4096;
     int sp=4095;
     string text;
     string inst,rd,rs,rt,addr,label,shamt,j;
     int rd_regno=0,rs_regno=0,rt_regno=0,j_addr_int=0;
     printreg(reg);
     pc=0;
     while(mem[pc].compare("00")!=0){
          instcount++;
          line=mem[pc];
          cout << pc << " = " << line << endl;
          stringstream parse(line);
          getline(parse, inst, ' ');
          if(inst.compare("add")==0){
               getline(parse, rd, ' ');
               getline(parse, rs, ' ');
               getline(parse, rt, ' ');
               rd_regno=reg_stoi(rd);
               rs_regno=reg_stoi(rs);
               rt_regno=reg_stoi(rt);
               reg[rd_regno]=reg[rs_regno]+reg[rt_regno];
               printreg(reg);
               totalcycle+=delays[0];
          }
          else if(inst.compare("sub")==0){
               getline(parse, rd, ' ');
               getline(parse, rs, ' ');
               getline(parse, rt, ' ');
               rd_regno=reg_stoi(rd);
               rs_regno=reg_stoi(rs);
               rt_regno=reg_stoi(rt);
               reg[rd_regno]=reg[rs_regno]-reg[rt_regno];
               printreg(reg);
               totalcycle+=delays[1];
          }
          else if(inst.compare("sll")==0){
               getline(parse, rd, ' ');
               getline(parse, rs, ' ');
               getline(parse, shamt, ' ');
               rd_regno=reg_stoi(rd);
               rs_regno=reg_stoi(rs);
               reg[rd_regno]=reg[rs_regno]*pow(2,stoi(shamt));
               printreg(reg);
               totalcycle+=delays[2];
          }
          else if(inst.compare("srl")==0){
               getline(parse, rd, ' ');
               getline(parse, rs, ' ');
               getline(parse, shamt, ' ');
               rd_regno=reg_stoi(rd);
               rs_regno=reg_stoi(rs);
               reg[rd_regno]=reg[rs_regno]/pow(2,stoi(shamt));
               printreg(reg);
               totalcycle+=delays[3];
          }
          else if(inst.compare("sw")==0){
               string offset;
               getline(parse, rs, ' ');
               getline(parse, offset, ' ');
               getline(parse, rd, ' ');
               rd_regno=reg_stoi(rd);
               rs_regno=reg_stoi(rs);
               mem[reg[rd_regno]+stoi(offset)]=to_string(reg[rs_regno]);
               printmem(mem);
               totalcycle+=delays[4];
          }
          else if(inst.compare("lw")==0){
               string offset;
               getline(parse, rs, ' ');
               getline(parse, offset, ' ');
               getline(parse, rd, ' ');
               rd_regno=reg_stoi(rd);
               rs_regno=reg_stoi(rs);
               reg[rs_regno]=stoi(mem[reg[rd_regno]+stoi(offset)]);
               printreg(reg);
               totalcycle+=delays[5];
          }
          else if(inst.compare("j")==0){
               getline(parse, addr, ' ');
               pc=stoi(addr)-1;
               totalcycle+=delays[6];
          }
          else if(inst.compare("jal")==0){
               getline(parse, addr, ' ');
               reg[31]=pc+1;
               // cout << reg[31] << endl;
               pc=stoi(addr)-1;
               totalcycle+=delays[7];
          }
          else if(inst.compare("jr")==0){
               // cout << reg[31] << endl;
               pc=reg[31]-1;
               totalcycle+=delays[8];
          }
          else if(inst.compare("beq")==0){
               getline(parse, rd, ' ');
               getline(parse, rs, ' ');
               getline(parse, addr, ' ');
               rd_regno=reg_stoi(rd);
               rs_regno=reg_stoi(rs);
               if(reg[rd_regno]==reg[rs_regno]){
                    pc=stoi(addr)-1;
               }
               totalcycle+=delays[9];
          }
          else if(inst.compare("bne")==0){
               getline(parse, rd, ' ');
               getline(parse, rs, ' ');
               getline(parse, addr, ' ');
               rd_regno=reg_stoi(rd);
               rs_regno=reg_stoi(rs);
               if(reg[rd_regno]!=reg[rs_regno]){
                    pc=stoi(addr)-1;
               }
               totalcycle+=delays[10];
          }
          else if(inst.compare("bgt")==0){
               getline(parse, rd, ' ');
               getline(parse, rs, ' ');
               getline(parse, addr, ' ');
               rd_regno=reg_stoi(rd);
               rs_regno=reg_stoi(rs);
               if(reg[rd_regno]>reg[rs_regno]){
                    pc=stoi(addr)-1;
               }
               totalcycle+=delays[11];
          }
          else if(inst.compare("bge")==0){
               getline(parse, rd, ' ');
               getline(parse, rs, ' ');
               getline(parse, addr, ' ');
               rd_regno=reg_stoi(rd);
               rs_regno=reg_stoi(rs);
               if(reg[rd_regno]>=reg[rs_regno]){
                    pc=stoi(addr)-1;
               }
               totalcycle+=delays[12];
          }
          else if(inst.compare("blt")==0){
               getline(parse, rd, ' ');
               getline(parse, rs, ' ');
               getline(parse, addr, ' ');
               rd_regno=reg_stoi(rd);
               rs_regno=reg_stoi(rs);
               if(reg[rd_regno]<reg[rs_regno]){
                    pc=stoi(addr)-1;
               }
               totalcycle+=delays[13];
          }
          else if(inst.compare("ble")==0){
               getline(parse, rd, ' ');
               getline(parse, rs, ' ');
               getline(parse, addr, ' ');
               rd_regno=reg_stoi(rd);
               rs_regno=reg_stoi(rs);
               if(reg[rd_regno]<=reg[rs_regno]){
                    pc=stoi(addr)-1;
               }
               totalcycle+=delays[14];
          }
          else if(inst.compare("bltz")==0){
               string offset;
               getline(parse, rd, ' ');
               getline(parse, offset, ' ');
               rd_regno=reg_stoi(rd);
               if(reg[rd_regno]<0){
                    pc=stoi(offset)-1;
               }
               totalcycle+=delays[14];
          }
          else if(inst.compare("bgtz")==0){
               string offset;
               getline(parse, rd, ' ');
               getline(parse, offset, ' ');
               rd_regno=reg_stoi(rd);
               if(reg[rd_regno]<=0){
                    pc=stoi(offset)-1;
               }
               totalcycle+=delays[14];
          }
          pc++;
     }
     file.close();
     cout << "Toal Cycles: " << totalcycle << endl;
     cout << "No of instruction executed: " << instcount << endl;
     cout << "Average instruction per cycle: " << (totalcycle/instcount) << endl;
     return 0;
}
int reg_stoi(string reg){
     // cout << reg << endl;
     int regno=-1;
     if(reg.compare("$zero")==0)
          regno=0;
     else if(reg.compare("$at")==0)
          regno=1;
     else if(reg.compare("$v0")==0)
          regno=2;
     else if(reg.compare("$v1")==0)
          regno=3;
     else if(reg.compare("$a0")==0)
          regno=4;
     else if(reg.compare("$a1")==0)
          regno=5;
     else if(reg.compare("$a2")==0)
          regno=6;
     else if(reg.compare("$a3")==0)
          regno=7;
     else if(reg.compare("$t0")==0)
          regno=8;
     else if(reg.compare("$t1")==0)
          regno=9;
     else if(reg.compare("$t2")==0)
          regno=10;
     else if(reg.compare("$t3")==0)
          regno=11;
     else if(reg.compare("$t4")==0)
          regno=12;
     else if(reg.compare("$t5")==0)
          regno=13;
     else if(reg.compare("$t6")==0)
          regno=14;
     else if(reg.compare("$t7")==0)
          regno=15;
     else if(reg.compare("$s0")==0)
          regno=16;
     else if(reg.compare("$s1")==0)
          regno=17;
     else if(reg.compare("$s2")==0)
          regno=18;
     else if(reg.compare("$s3")==0)
          regno=19;
     else if(reg.compare("$s4")==0)
          regno=20;
     else if(reg.compare("$s5")==0)
          regno=21;
     else if(reg.compare("$s6")==0)
          regno=22;
     else if(reg.compare("$s7")==0)
          regno=23;
     else if(reg.compare("$t8")==0)
          regno=24;
     else if(reg.compare("$t9")==0)
          regno=25;
     else if(reg.compare("$k0")==0)
          regno=26;
     else if(reg.compare("$k1")==0)
          regno=27;
     else if(reg.compare("$gp")==0)
          regno=28;
     else if(reg.compare("$sp")==0)
          regno=29;
     else if(reg.compare("$fp")==0)
          regno=30;
     else if(reg.compare("$ra")==0)
          regno=31;
     // cout << "=" << regno << endl;
     return regno;
}
void initialize_delays(int delays[]){
     for(int i=0; i<15; i++){
          delays[i]=0;
     }
}
void initialize_reg(int reg[]){
     for(int i=0; i<32; i++){
          reg[i]=0;
     }
}
void initialize_mem(string mem[]){
     for(int i=0; i<4096; i++){
          mem[i]="00";
     }
}
void printreg(int reg[]){
     cout << "------------------------------------------------------------------------------" << endl;
     for(int i=0; i<32; i=i+4){
          cout << i << " = " << reg[i] << " " << (i+1) << " = " << reg[i+1] << " " << (i+2) << " = " << reg[i+2] << " " << (i+3) << " = " << reg[i+3] << endl;
     }
     cout << "------------------------------------------------------------------------------" << endl;
}
void printmem(string mem[]){
     cout << "###############################################################################" << endl;
     for(int i=0; i<4096; i=i+4){
          cout << i << "=" << mem[i] << " " << (i+1) << "=" << mem[i+1] << " " << (i+2) << "=" << mem[i+2] << " " << (i+3) << "=" << mem[i+3] << endl;
     }
     cout << "###############################################################################" << endl;
}