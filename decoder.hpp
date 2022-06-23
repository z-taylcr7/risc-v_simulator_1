//
// Created by Cristiano on 2022/6/21.
//
#include <map>
#include "utility.hpp"
#ifndef RISC_V_SIMULATOR_DECODER_HPP
#define RISC_V_SIMULATOR_DECODER_HPP
extern uint reg[32];
extern uint mem[500005];
std::map<int,std::string> instruction;
unsigned int PC = 0;
unsigned int last=0;
namespace Cristiano {
    enum codeType {
        U, J, I, B, S, R
    };

    codeType judgeType(const std::string &o) {
        if (o == "0110111" || o == "0010111")return U;
        if (o == "1101111")return J;
        if (o == "1100111" || o == "0010011")return I;
        if (o == "0000011")return I;
        if (o == "1100011")return B;
        if (o == "0100011")return S;
        if (o == "0110011")return R;
        return R;
    };
    enum opcode {
        add, sub, sll, slt, sltu, Xor, srl, sra, Or, And,//R
        lui, auipc,//U
        jal, beq, bne, blt, bge, bltu, bgeu,//B
        lb, lh, lw, lbu, lhu, addi, slti, sltiu, xori, ori, andi, slli, srli, srai,//I
        jalr,//J
        sb, sh, sw//S
    };

    std::string to8(const char &i1, const char &i2) {
        int in1, in2;
        if (i1 >= 'A' && i1 <= 'F')in1 = i1 - 'A' + 10;
        else in1 = i1 - '0';
        if (i2 >= 'A' && i2 <= 'F')in2 = i2 - 'A' + 10;
        else in2 = i2 - '0';
        in1 = 16 * in1 + in2;
        int cnt = 7;
        char ch[8];
        while (cnt >= 0) {
            ch[cnt--] = (char) ('0' + (in1) % 2);
            in1 >>= 1;
        }

        std::string str=ch;
        return str;
    }
    uint readMemory(int pc){
        uint x=0;
        for(int i=0;i<4;i++){
            x|=mem[pc+i]<<(i<<3);
        }
        return x;
    }
    class code {
    private:
        std::string order;

    public:
        int height=0;
        int memo=0;uint data=0;
        int regi=0;uint tmp=0;int pos=0;
        unsigned int rs1, rs2;
        unsigned int rd;
        int imm;int shamt;
        codeType type;
        opcode command;

        code() {
            for (int i = 0; i < 31; i++)order[i] = '0';
        }

        code(const std::string &str) : order(reverse(str)) {
            memo=regi=0;
        }


    private:
        void Rdecode() {
            rd = btoi(order.substr(20, 5));
            rs1 = btoi(order.substr(12, 5));
            rs2 = btoi(order.substr(7, 5));
            std::string fun(order.substr(17, 3));
            int x = order[1] - '0';
            if (fun == "000") {
                command = x ? sub : add;
            }
            if (fun == "001")command = sll;
            if (fun == "010")command = slt;
            if (fun == "011")command = sltu;
            if (fun == "100")command = Xor;
            if (fun == "101") {
                command = x ? sra : srl;
            }
            if (fun == "110")command = Or;
            if (fun == "111")command = And;
        }

        void Jdecode() {
            rd = btoi(order.substr(20, 5));
            imm = btoi(order[0]+  order.substr(12, 8)+ order[11] + order.substr(1, 10)+ '0' );
            command = jal;
            height=20;
            if(imm>=1<<20)imm-=1<<21;
        }

        void Idecode() {
            rd = btoi(order.substr(20, 5));
            rs1 = btoi(order.substr(12, 5));
            imm = btoi(order.substr(0, 12));
            if(imm>=2048)imm-=4096;
            height=11;
            if (order.substr(25, 7) == "1100111") {
                command = jalr;
            } else {
                if (order.substr(25, 7) == "0010011") {
                    std::string fun(order.substr(17, 3));
                    if (fun == "000")command = addi;
                    if (fun == "010")command = slti;
                    if (fun == "011")command = sltiu;
                    if (fun == "100")command = xori;
                    if (fun == "110")command = ori;
                    if (fun == "111")command = andi;
                    if (fun == "001"){
                        shamt=btoi(order.substr(7,5));command = slli;
                    }
                    if (fun == "101") {
                        if (order[1] == '1')command = srai;
                        else {
                            command = srli;
                        }
                        shamt=btoi(order.substr(7,5));
                    }
                } else {
                    std::string fun(order.substr(17, 3));
                    if (fun == "000")command = lb;
                    if (fun == "001")command = lh;
                    if (fun == "010")command = lw;
                    if (fun == "100")command = lbu;
                    if (fun == "101")command = lhu;
                }
            }
        }

        void Bdecode() {
            rs1 = btoi(order.substr(12, 5));
            rs2 = btoi(order.substr(7, 5));
            std::string str=order[24]+order.substr(1, 6)+order.substr(20, 4)  +'0';
            imm = btoi(order[0]+str);
            if(imm>=4096)imm-=8192;
            height=12;
            std::string fun(order.substr(17, 3));
            if (fun == "000")command = beq;
            if (fun == "001")command = bne;
            if (fun == "100")command = blt;
            if (fun == "101")command = bge;
            if (fun == "110")command = bltu;
            if (fun == "111")command = bgeu;
        }

        void Sdecode() {
            rs1 = btoi(order.substr(12, 5));
            rs2 = btoi(order.substr(7, 5));
            imm = btoi( (order.substr(0, 7))+ order.substr(20, 5));
            if(imm>=2048)imm-=4096;
            height=11;
            std::string fun(order.substr(17, 3));
            if (fun == "000")command = sb;
            if (fun == "001")command = sh;
            if (fun == "010")command = sw;
        };

        void Udecode() {
            if (order[26] == '0')command = auipc;
            else command = lui;
            rd = btoi(order.substr(20, 5));
            imm = btoi( order.substr(0, 20));if(imm>=1<<19)imm-=1<<20;//12x0
            height=31;
        }

        //execute
        void ADD() {
            unsigned int cur1 = reg[rs1];
            unsigned int cur2 = reg[rs2];
            unsigned int cur = cur1 + cur2;
            data = cur;
            regi=1;PC+=4;

        }

        void SUB() {
            //           bool sign=(reg[rs1]>=reg[rs2]);
            int cur1 = reg[rs1];
            int cur2 = reg[rs2];
            int cur(cur1 - cur2);
            data = cur;
            regi=1;PC+=4;


        }

        void XOR() {
             int cur1(reg[rs1]);
             int cur2(reg[rs2]);
            int cur(cur1 ^ cur2);
            data = cur;
            regi=1;PC+=4;

        }

        void AND() {
             int cur1(reg[rs1]);
             int cur2(reg[rs2]);
            int cur(cur1 & cur2);
            data = cur;
            regi=1;PC+=4;

        }
        void OR(){
            unsigned int cur1(reg[rs1]);
            unsigned int cur2(reg[rs2]);
            int cur(cur1 | cur2);
            data = cur;
            regi=1;PC+=4;

        }
        //B
        void BEQ(){
            if(reg[rs1]==reg[rs2]){
                PC+=sext(imm, height);
            }else PC+=4;
        }
        void BGE(){
            int cur1=reg[rs1];
            int cur2=reg[rs2];
            if(cur1>=cur2){
                PC+=sext(imm, height);
            }else{
                PC+=4;
            }

        }
        void BGEU(){

            unsigned int cur1=reg[rs1];
            unsigned int cur2=reg[rs2];
            if(cur1>=cur2){
                PC+=sext(imm, height);
            }else{
                PC+=4;
            }
        }
        void BLT(){
            int cur1=reg[rs1];
            int cur2=reg[rs2];
            if(cur1<cur2){
                PC+=sext(imm, height);
            }else{
                PC+=4;
            }
        }
        void BLTU(){
            unsigned int cur1=reg[rs1];
            unsigned int cur2=reg[rs2];
            if(cur1<cur2){
                PC+=sext(imm, height);
            }else{
                PC+=4;
            }
        }
        void BNE(){
            if(reg[rs1]!=reg[rs2]){
                PC+=sext(imm, height);
            }else{
                PC+=4;
            }
        }
        void JAL(){
            data=PC+4;PC+=sext(imm, height);
            regi=1;
        }
        //J
        void JALR(){
            int t=PC+4;
            PC=(reg[rs1]+sext(imm, height))&~1;
            data=t;
            regi=1;
        }
        //U
        void AUIPC() {
            data = PC + reg[rs1];
            regi=1;PC+=4;
        }

        //I
        void ADDI() {
            data = reg[rs1] + sext(imm, height);
            regi=1;PC+=4;

        }

        void ANDI() {
            int cur(reg[rs1]);
            data = cur & sext(imm, height);
            regi=1;PC+=4;

        }
        void ORI() {
            int cur(reg[rs1]);
            data = cur | sext(imm, height);
            regi=1;PC+=4;

        }
        void XORI() {
            int cur(reg[rs1]);
            data = cur ^ sext(imm, height);
            regi=1;PC+=4;

        }
        void LB(){
            int x= readMemory(reg[rs1]+sext(imm, height));
            data=sext(x-((x>>8)<<8),7);
            regi=1;PC+=4;
        }
        void LBU(){
            int x=readMemory(reg[rs1]+sext(imm, height));
            data=x-((x>>8)<<8);
            regi=1;PC+=4;
        }
        void LH(){
            int x=readMemory(reg[rs1]+sext(imm, height));
            data=sext(x-((x>>16)<<16),15);
            regi=1;PC+=4;
        }
        void LHU(){
            int x=readMemory(reg[rs1]+sext(imm, height));
            data=x-((x>>16)<<16);
            regi=1;PC+=4;
        }
        void LW(){
            int x=readMemory(reg[rs1]+sext(imm, height));
            data=x;
            regi=1;PC+=4;
        }
        void LUI(){
 //todo:
 height=31;
            data = imm << 12;
            regi=1;PC+=4;
        }
        void SB(){
            memo=1;tmp=reg[rs2]-((reg[rs2]>>8)<<8);pos=reg[rs1]+sext(imm, height);PC+=4;
        }
        void SH(){
            memo=1;pos=reg[rs1]+sext(imm, height);tmp=reg[rs2]-((reg[rs2]>>16)<<16);PC+=4;
        }
        void SW(){
            memo=1;tmp=reg[rs2];pos=reg[rs1]+sext(imm, height);PC+=4;
        }
        void SLL(){
            data=reg[rs1]<<reg[rs2];
            regi=1;PC+=4;
        }
        void SLLI(){
            data=reg[rs1]<<shamt;
            regi=1;PC+=4;
        }
        void SLT(){
            int a1=reg[rs1];int a2=reg[rs2];
            data=(a1<a2)?1:0;
            regi=1;PC+=4;
        }
        void SLTI(){
            int a1=reg[rs1];int a2=imm;
            data=(a1<a2)?1:0;
            regi=1;PC+=4;
        }
        void SLTIU(){
            unsigned int a1=reg[rs1];unsigned int a2=imm;
            data=(a1<a2)?1:0;
            regi=1;PC+=4;
        }
        void SLTU(){
            unsigned int a1=reg[rs1];unsigned int a2=reg[rs2];
            data=(a1<a2)?1:0;
            regi=1;PC+=4;
        }
        void SRA(){
            uint a1=reg[rs1],a2=reg[rs2];
            data=a1>>a2;
            regi=1;PC+=4;
        }
        void SRAI(){
            data=reg[rs1]>>(shamt);
            regi=1;PC+=4;
        }
        void SRL(){
            data=reg[rs1]>>(reg[rs2]);
            regi=1;PC+=4;
        }
        void SRLI(){
            data=reg[rs1]>>(unsigned int)shamt;
            regi=1;PC+=4;
        }

    public:
        void execute() {
            if (command == add)ADD();
            if (command == sub)SUB();
            if (command == Xor)XOR();
            if (command == And)AND();
            if (command == addi)ADDI();
            if (command == andi)ANDI();
            if (command == xori)XORI();
            if (command == auipc)AUIPC();
            if (command == beq)BEQ();
            if (command == bne)BNE();
            if (command == blt)BLT();
            if (command == bge)BGE();
            if (command == bltu)BLTU();
            if (command == bgeu)BGEU();
            if (command == ori)ORI();
            if (command == jalr)JALR();
            if (command == jal)JAL();
            if (command == lb)LB();
            if (command == lh)LH();
            if (command == lw)LW();
            if (command == lbu)LBU();
            if (command == lhu)LHU();
            if (command == Or)OR();
            if (command == lui)LUI();
            if (command == sb)SB();
            if (command == sh)SH();
            if (command == sw)SW();
            if (command == srl)SRL();
            if (command == sra)SRA();
            if (command==sll)SLL();
            if (command==slli)SLLI();
            if (command == srli)SRLI();
            if (command == srai)SRAI();
            if (command == slti)SLTI();
            if (command == slt)SLT();
            if (command == sltu)SLTU();
            if (command == sltiu)SLTIU();
        }
       int fetchCode() {//end return false
            if(PC>last)return -1;
            char i1, i2;
            std::string line = "";
            for (int i = PC; i < PC+4; i++) {
                i1 = instruction[i][0];
                i2 = instruction[i][1];
                line.append(reverse(to8(i1, i2)).c_str());
            }
            order= reverse(line);
            if(order=="00001111111100000000010100010011")return 1;
            return 0;
        }
        void decode() {
            std::string opt = order.substr(25, 7);
            type = judgeType(opt);
            if (type == R)Rdecode();
            if (type == J)Jdecode();
            if (type == I)Idecode();
            if (type == S)Sdecode();
            if (type == U)Udecode();
            if (type == B)Bdecode();
        }
        void writeMemory(){
           if(memo==1){
               for(int i=0;i<4;i++){
                   mem[pos]=tmp&0xff;
                   tmp>>=8;pos++;
               }
               tmp=pos=0;memo=0;
           }
        }
        void writeRegister(){
            if(regi==1){
                reg[rd]=data;data=0;regi=0;
            }
        }


    };
    

    void read() {
        std::string str,strmem;
        std::cin>>str;
        int cnt=0,k=0;
        while(str=="end"||!std::cin.eof()){
            if(str[0]=='@'){
                str=str.substr(1);cnt=xtoi(str);
            }
            else{
                int num=(hexStringToDec(str));
                mem[cnt]|=num;
                instruction[cnt]=str;cnt++;
            }
            std::cin>>str;
            if(std::cin.eof())break;
            if(str=="end"){
                last=cnt;break;
            }
        }
    }



};


#endif //RISC_V_SIMULATOR_DECODER_HPP
