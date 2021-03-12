#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sstream>
#include <map>
using namespace std;
ifstream file,file1;
ofstream file2,file3;

int reg[32], pc, clk, done, inst_count, number_of_instructions_executed;
string mem[4096];
map<string, int> labels;

// temporary strings for parsing
string inst_ID1, rs_ID, rt_ID, rd_ID, label_ID, offset_ID, line, r_sw_ID;
stringstream buffer_if, buffer_id, buffer_exe, buffer_mem, buffer_wb;

void print_reg(int[]);
void print_mem(string[]);
int get_addr_of_label(string);
int reg_stoi(string);
void initialize_reg();
void initialize_mem();
string reg_itos[32] = {"$zero", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3", "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7", "$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra"};;
int last_updated[32] = {-100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100};

int add(int a, int b) {return a + b;}
int sub(int a, int b) {return a - b;}
int sll(int a, int b) {return (int) ((unsigned int)a << b);}
int srl(int a, int b) {return (int)((unsigned int)a >> b);}
int eq(int a, int b) {return a == b;}
int neq(int a, int b) {return a != b;}
int lez(int a, int b) {return a <= 0;}
int gez(int a, int b) {return a >= 0;}
int get_op(string);
int (*alu_op[])(int, int) = {add, sub, sll, srl, eq, neq, lez, gez};

class IF_ID_latch {
public:
	int done_; //control signals
	string instruction; //data
	int instruction_number, stall, stalled, c_branch_stall;//stall signal

	IF_ID_latch() {}
};

class ID_EX_latch {
public:
	bool lw, sw, reg_wr, c_branch; //control signals
	int op, done_; //control signals
	int op1, op2, target, wr_data, regd; //data (, pc)
	int instruction_number;

	ID_EX_latch() {}
};

class EX_MEM_latch {
public:
	bool lw, sw, reg_wr, done_; //control signals
	int addr, wr_data, regd; //data (, pc)
	int instruction_number;

	EX_MEM_latch() {}
};

class MEM_WB_latch {
public:
	bool reg_wr, done_; //control signals
	int regd, wr_data; //data
	int instruction_number;

	MEM_WB_latch() {}
};

class PipeLine {
public:
	IF_ID_latch *if_id_latch, *temp_if_id_latch;
	ID_EX_latch *id_ex_latch, *temp_id_ex_latch;
	EX_MEM_latch *ex_mem_latch, *temp_ex_mem_latch;
	MEM_WB_latch *mem_wb_latch, *temp_mem_wb_latch;

	void fetch() {
		/* Loads the instruction pointed by pc */
		if (if_id_latch != nullptr && if_id_latch->stall) {
			buffer_if << "IF Idle" << endl;
			return;
		}

		if (if_id_latch != nullptr && if_id_latch->c_branch_stall) {
			buffer_if << "IF Stalling" << endl;
			return;
		}

		temp_if_id_latch = new IF_ID_latch();
		
		if (mem[pc].compare("00") == 0) { // terminate condition
			buffer_if << "IF Sending done signal" << endl;
			temp_if_id_latch->done_ = 1;
			temp_if_id_latch->stall = 0;
			temp_if_id_latch->c_branch_stall = 0;
			return;
		}

		buffer_if << "IF fetching instruction " << pc << "-> " << mem[pc] << endl;
		temp_if_id_latch->stall = 0;
		temp_if_id_latch->stalled = 0;
		temp_if_id_latch->c_branch_stall = 0;
		temp_if_id_latch->done_ = 0;
		temp_if_id_latch->instruction_number = pc;
		temp_if_id_latch->instruction = mem[pc++];
	}

	void decode() {
		/* 	Decodes all the required information from the instruction 
			If the instuction is an unconditional branch then updates pc */
		if (if_id_latch == nullptr) {
			buffer_id << "ID Idle" << endl;
			return;
		}
		
		if (if_id_latch->stall) {
			buffer_id << "ID Stalling" << endl;
			temp_id_ex_latch = nullptr;
			return;
		}

		temp_id_ex_latch = new ID_EX_latch();

		if (if_id_latch->done_) { // terminate condition
			buffer_id << "ID Sending done signal" << endl;
			temp_id_ex_latch->done_ = 1;
			return;
		}
		
		//Temporary variable to check whether this instruction is reading from registers or not
		int temp_rs = -1, temp_rt = -1, temp_r_sw = -1;

		buffer_id << "ID decoding instruction " << if_id_latch->instruction_number << "-> " << if_id_latch->instruction << endl;
		stringstream parse(if_id_latch->instruction);
		getline(parse, inst_ID1, ' ');
		//updating control signals
		temp_id_ex_latch->done_ = 0;
		temp_id_ex_latch->op = get_op(inst_ID1);
		temp_id_ex_latch->lw = inst_ID1.compare("lw") == 0;
		temp_id_ex_latch->sw = inst_ID1.compare("sw") == 0;
		temp_id_ex_latch->reg_wr = inst_ID1.compare("add") == 0 ||
									inst_ID1.compare("sub") == 0 ||
									inst_ID1.compare("sll") == 0 ||
									inst_ID1.compare("srl") == 0 ||
									inst_ID1.compare("lw") == 0;
		temp_id_ex_latch->c_branch = inst_ID1.compare("beq") == 0 ||
									inst_ID1.compare("bne") == 0 ||
									inst_ID1.compare("blez") == 0 ||
									inst_ID1.compare("bgtz") == 0;

		temp_id_ex_latch->instruction_number = if_id_latch->instruction_number;
		//updating data op1, op2, regd, target, pc, wr_data
		if (inst_ID1.compare("add") == 0 || inst_ID1.compare("sub") == 0 || inst_ID1.compare("sll") == 0 || inst_ID1.compare("srl") == 0) {
			getline(parse, rd_ID, ' ');
			getline(parse, rs_ID, ' ');
			temp_rs = reg_stoi(rs_ID);
			getline(parse, rt_ID, ' ');
			temp_rt = reg_stoi(rt_ID);
			temp_id_ex_latch->regd = reg_stoi(rd_ID);
			temp_id_ex_latch->op1 = reg[reg_stoi(rs_ID)];
			temp_id_ex_latch->op2 = reg[reg_stoi(rt_ID)];
		}
		else if (inst_ID1.compare("sw")==0){
			getline(parse, r_sw_ID, ' ');
			temp_r_sw = reg_stoi(r_sw_ID);
			getline(parse, offset_ID, ' ');
			getline(parse, rt_ID, ' ');
			temp_rt = reg_stoi(rt_ID);
			temp_id_ex_latch->wr_data = reg[reg_stoi(r_sw_ID)];
			if (reg_stoi(offset_ID) != -1) {
				temp_id_ex_latch->op1 = reg[reg_stoi(offset_ID)];
				temp_rs = reg_stoi(offset_ID);
			}
			else
				temp_id_ex_latch->op1 = stoi(offset_ID);
			temp_id_ex_latch->op2 = reg[reg_stoi(rt_ID)];
		}
		else if(inst_ID1.compare("lw")==0){
			getline(parse, rd_ID, ' ');
			getline(parse, offset_ID, ' ');
			getline(parse, rt_ID, ' ');
			temp_rt = reg_stoi(rt_ID);
			temp_id_ex_latch->regd = reg_stoi(rd_ID);
			if (reg_stoi(offset_ID) != -1) {
				temp_id_ex_latch->op1 = reg[reg_stoi(offset_ID)];
				temp_rs = reg_stoi(offset_ID);
			}
			else
				temp_id_ex_latch->op1 = stoi(offset_ID);
			temp_id_ex_latch->op2 = reg[reg_stoi(rt_ID)];
		}
		else if (inst_ID1.compare("beq") == 0 || inst_ID1.compare("bne") == 0) {
			getline(parse, rs_ID, ' ');
			temp_rs = reg_stoi(rs_ID);
			getline(parse, rt_ID, ' ');
			temp_rt = reg_stoi(rt_ID);
			getline(parse, label_ID, ' ');
			temp_id_ex_latch->target = labels[label_ID];
			temp_id_ex_latch->op1 = reg[reg_stoi(rs_ID)];
			temp_id_ex_latch->op2 = reg[reg_stoi(rt_ID)];
		}
		else if (inst_ID1.compare("blez") == 0 || inst_ID1.compare("bgtz") == 0) {
			getline(parse, rs_ID, ' ');
			temp_rs = reg_stoi(rs_ID);
			getline(parse, label_ID, ' ');
			temp_id_ex_latch->target = labels[label_ID];
			temp_id_ex_latch->op1 = reg_stoi(rs_ID);
		}
		else if(inst_ID1.compare("j")==0){
			getline(parse, label_ID, ' ');
			pc = labels[label_ID];
		}
		else if(inst_ID1.compare("jal")==0){
			getline(parse, label_ID, ' ');
			reg[31] = pc;
			pc = labels[label_ID];
		}
		else if(inst_ID1.compare("jr")==0){
			getline(parse, rs_ID, ' ');
			temp_rs = reg_stoi(rs_ID);
			if (last_updated[temp_rs] < clk - 2) pc = reg[reg_stoi(rs_ID)];
		}
		else {
			cout << inst_ID1 << "HERE ";
			buffer_id << "INVALID INSTRUCTION" << endl;
			exit(0);
		}
		if (!if_id_latch->stalled && (temp_rs != -1 || temp_rt != -1 || temp_r_sw != -1)) {
			/* if this is a register that we have to read from and the 
			update may not be complete so ID stage stalls for required cycles */
			int stall_for_rs = 0, stall_for_rt = 0, stall_for_r_sw = 0;
			if (temp_rs != -1) {
				if (clk - last_updated[temp_rs] == 2) 
					stall_for_rs = 1;
				else if (clk - last_updated[temp_rs] == 1)
					stall_for_rs = 2;
			}
			if (temp_rt != -1) {
				if (clk - last_updated[temp_rt] == 2) 
					stall_for_rt = 1;
				else if (clk - last_updated[temp_rt] == 1)
					stall_for_rt = 2;
			}
			if (temp_r_sw != -1) {
				if (clk - last_updated[temp_r_sw] == 2) 
					stall_for_r_sw = 1;
				else if (clk - last_updated[temp_r_sw] == 1)
					stall_for_r_sw = 2;
			}
			int stall_ = max(max(stall_for_rs, stall_for_rt), stall_for_r_sw);
			if_id_latch->stall = stall_;
			if (stall_) {
				buffer_id << "ID stalling (" << stall_ << " cycles)" << endl;
				temp_id_ex_latch = nullptr;
				return;
			}
		}

		if (temp_id_ex_latch->c_branch) {
			//If it is a conditional branch then IF stage stalls for 1 cycle
			buffer_id << "This is a conditional Branch" << endl;
			if_id_latch->c_branch_stall = 1;
		}

		if (temp_id_ex_latch->reg_wr) {
			//Update the last_updated array for the register we have to write to
			last_updated[temp_id_ex_latch->regd] = clk;
		}
	}

	void execute() {
		if (id_ex_latch == nullptr) {
			buffer_exe << "EXE Idle" << endl;
			return;
		}

		temp_ex_mem_latch = new EX_MEM_latch(); 

		if (id_ex_latch->done_) {  // terminate condition
			buffer_exe << "EXE Sending done signal" << endl;
			temp_ex_mem_latch->done_ = 1;
			return;
		}
		buffer_exe << "EXE processing instruction "<< id_ex_latch->instruction_number << "-> " << mem[id_ex_latch->instruction_number] << endl;
		//updating control signals
		temp_ex_mem_latch->done_ = 0;
		temp_ex_mem_latch->lw = id_ex_latch->lw;
		temp_ex_mem_latch->sw = id_ex_latch->sw;
		temp_ex_mem_latch->reg_wr = id_ex_latch->reg_wr;

		temp_ex_mem_latch->instruction_number = id_ex_latch->instruction_number;
		//updating data
		int res = 0;
		temp_ex_mem_latch->wr_data = id_ex_latch->wr_data;
		temp_ex_mem_latch->regd = id_ex_latch->regd;
		if (id_ex_latch->op != -1) {
			res = (alu_op[id_ex_latch->op])(id_ex_latch->op1, id_ex_latch->op2);
		}
		if (id_ex_latch->lw || id_ex_latch->sw)
			temp_ex_mem_latch->addr = res;
		if (id_ex_latch->c_branch && res) {
			pc = id_ex_latch->target;
			buffer_exe << "\tBranching to: " << mem[pc] << endl;
		}
		if (id_ex_latch->reg_wr) {
			temp_ex_mem_latch->wr_data = res;
		}
	}

	void ma() {
		if (ex_mem_latch == nullptr) {
			buffer_mem << "MEM Idle" << endl;
			return;
		}

		temp_mem_wb_latch = new MEM_WB_latch();

		if (ex_mem_latch->done_) { // terminate condition
			buffer_mem << "MEM Sending done signal" << endl;
			temp_mem_wb_latch->done_ = 1;
			return;
		}
		buffer_mem << "MEM processing instruction " << ex_mem_latch->instruction_number << "-> " << mem[ex_mem_latch->instruction_number] << endl;
		//updating control signals
		temp_mem_wb_latch->done_ = 0;
		temp_mem_wb_latch->reg_wr = ex_mem_latch->reg_wr;
		temp_mem_wb_latch->instruction_number = ex_mem_latch->instruction_number;
		//updating data
		temp_mem_wb_latch->regd = ex_mem_latch->regd;
		temp_mem_wb_latch->wr_data = ex_mem_latch->wr_data;
		if (ex_mem_latch->sw) {
			buffer_mem << "\tStoring " << ex_mem_latch->wr_data << " to memory location " << ex_mem_latch->addr << endl;
			mem[ex_mem_latch->addr] = to_string(ex_mem_latch->wr_data);
		}
		if (ex_mem_latch->lw) {
			temp_mem_wb_latch->wr_data = stoi(mem[ex_mem_latch->addr]);
			buffer_mem << "\tLoading " << temp_mem_wb_latch->wr_data << " from memory location " << ex_mem_latch->addr << endl;
		}
	}

	void wb() {
		if (mem_wb_latch == nullptr) {
			buffer_wb << "WB Idle" << endl;
			return;
		}
		if (mem_wb_latch->done_) { // terminate condition
			buffer_wb << "WB Sending done signal" << endl;
			done = 1;
			return;
		}
		++number_of_instructions_executed;
		buffer_wb << "WB processing instruction " << mem_wb_latch->instruction_number << "-> " << mem[mem_wb_latch->instruction_number] << endl;
		if (mem_wb_latch->reg_wr) {
			buffer_wb << "\t writing " << mem_wb_latch->wr_data << " to register " << reg_itos[mem_wb_latch->regd] << endl;
			reg[mem_wb_latch->regd] = mem_wb_latch->wr_data;
		}
	}

	void buffers(int print) {
		if (print) {
			file3 << buffer_wb.str() << buffer_mem.str() << buffer_exe.str() << buffer_id.str() << buffer_if.str();
			return;
		}
		buffer_if.str("");
		buffer_id.str("");
		buffer_exe.str("");
		buffer_mem.str("");
		buffer_wb.str("");
	}

	PipeLine() {
		//Initialize pc, clk, terminate
		pc = 0, clk = 0, done = 0, inst_count = 0, number_of_instructions_executed = 0;

		//Initialize latches
		if_id_latch = nullptr;
		id_ex_latch = nullptr;
		ex_mem_latch = nullptr;
		mem_wb_latch = nullptr;
		temp_if_id_latch = nullptr;
		temp_id_ex_latch = nullptr;
		temp_ex_mem_latch = nullptr;
		temp_mem_wb_latch = nullptr;

		//Processing the input program
		while (1) {
			++clk;
			file3 << "Clock cycle " << clk << endl;
			buffers(0);
			wb();
			decode();
			execute();
			ma();
			fetch();
			buffers(1);
			if (if_id_latch != nullptr && if_id_latch->stall) {
				--if_id_latch->stall;
				if (if_id_latch->stall == 0)
					if_id_latch->stalled = 1;
			} else if (if_id_latch != nullptr && if_id_latch->c_branch_stall) {
				--if_id_latch->c_branch_stall;
				if_id_latch = temp_if_id_latch;
			} else {
				if_id_latch = temp_if_id_latch;
			}
			id_ex_latch = temp_id_ex_latch;
			ex_mem_latch = temp_ex_mem_latch;
			mem_wb_latch = temp_mem_wb_latch;
			temp_if_id_latch = nullptr;
			temp_id_ex_latch = nullptr;
			temp_ex_mem_latch = nullptr;
			temp_mem_wb_latch = nullptr;
			if (done) {
				int cycles_for_execution = clk - 1;
				file2 << "Toal Cycles: " << cycles_for_execution << endl;
				file2 << "No of instruction executed: " << number_of_instructions_executed << endl;
				file2 << "Average instruction per cycle: " << (number_of_instructions_executed/(double)cycles_for_execution) << endl;
				break;
			}
		}
	}

};

int main() {
	file.open("input.txt");
	file2.open("result.txt");
	file3.open("output_pipeline_stages.txt");

	//----Initialising Register array and Memory file with zeroes ----
	initialize_reg();
	initialize_mem();

	//--------------- Loading Program into memory ---------------
	getline(file, line);
	if (line.at(line.size() - 1) == ':') {
		labels[line.substr(0, line.size() - 1)] = pc;
		getline(file, line);
	}
	while (file) {
		mem[pc++] = line;
		getline(file, line);
		if (line.at(line.size() - 1) == ':') {
			labels[line.substr(0, line.size() - 1)] = pc;
			getline(file, line);
		}
	}

	//-------------- Test Case --------------
	reg[reg_stoi("$a1")] = 1;
	reg[reg_stoi("$at")] = 1;
	reg[reg_stoi("$v1")] = 10;
	reg[reg_stoi("$a3")] = 10;

	PipeLine pl;

	return 0;
}

void initialize_reg(){
	for(int i = 0; i < 32; ++i)
		reg[i]=0;
	reg[29] = 4096;
}

void initialize_mem() {
	for (int i = 0; i < 4096; ++i)
		mem[i]="00";
}

int get_op(string s) {
	if (s.compare("add") == 0 || s.compare("lw") == 0 || s.compare("sw") == 0) return 0;
	if (s.compare("sub") == 0) return 1;
	if (s.compare("sll") == 0) return 2;
	if (s.compare("srl") == 0) return 3;
	if (s.compare("beq") == 0) return 4;
	if (s.compare("bne") == 0) return 5;
	if (s.compare("blez") == 0) return 6;
	if (s.compare("bgtz") == 0) return 7;
	return -1;
}

int reg_stoi(string reg_){
	if (reg_.compare("$zero")==0) return 0;
	if (reg_.compare("$at")==0) return 1;
	if (reg_.compare("$v0")==0) return 2;
	if (reg_.compare("$v1")==0) return 3;
	if (reg_.compare("$a0")==0) return 4;
	if (reg_.compare("$a1")==0) return 5;
	if (reg_.compare("$a2")==0) return 6;
	if (reg_.compare("$a3")==0) return 7;
	if (reg_.compare("$t0")==0) return 8;
	if (reg_.compare("$t1")==0) return 9;
	if (reg_.compare("$t2")==0) return 10;
	if (reg_.compare("$t3")==0) return 11;
	if (reg_.compare("$t4")==0) return 12;
	if (reg_.compare("$t5")==0) return 13;
	if (reg_.compare("$t6")==0) return 14;
	if (reg_.compare("$t7")==0) return 15;
	if (reg_.compare("$s0")==0) return 16;
	if (reg_.compare("$s1")==0) return 17;
	if (reg_.compare("$s2")==0) return 18;
	if (reg_.compare("$s3")==0) return 19;
	if (reg_.compare("$s4")==0) return 20;
	if (reg_.compare("$s5")==0) return 21;
	if (reg_.compare("$s6")==0) return 22;
	if (reg_.compare("$s7")==0) return 23;
	if (reg_.compare("$t8")==0) return 24;
	if (reg_.compare("$t9")==0) return 25;
	if (reg_.compare("$k0")==0) return 26;
	if (reg_.compare("$k1")==0) return 27;
	if (reg_.compare("$gp")==0) return 28;
	if (reg_.compare("$sp")==0) return 29;
	if (reg_.compare("$fp")==0) return 30;
	if (reg_.compare("$ra")==0) return 31;
	return -1;
}

void printreg(int reg_[]){
	cout << "------------------------------------------------------------------------------" << endl;
	for(int i=0; i<32; i=i+4){
		cout << i << " = " << reg_[i] << " " << (i+1) << " = " << reg_[i+1] << " " << (i+2) << " = " << reg_[i+2] << " " << (i+3) << " = " << reg_[i+3] << endl;
	}
	cout << "------------------------------------------------------------------------------" << endl;
}
void printmem(string mem_[]){
	cout << "###############################################################################" << endl;
	for(int i=0; i<4096; i=i+4){
		cout << i << "=" << mem_[i] << " " << (i+1) << "=" << mem_[i+1] << " " << (i+2) << "=" << mem_[i+2] << " " << (i+3) << "=" << mem_[i+3] << endl;
	}
	cout << "###############################################################################" << endl;
}