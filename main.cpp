

#include <iomanip>
#include <unordered_map>
#include <bitset>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <iomanip>
#include <string>
#include <sstream>
using namespace std;


bool isBreak;
bool noBranch = true;

struct postThings{
    int instr=0, value=0;
};
postThings postALU, postMEM;

struct instruction{
    int v,rs,rd,rt,shamt, func, opcode, imm, target, intVal, addr, jTarget,
        dest=-1, src1=-1, src2=-1;
    string out, bitstr, istr;
    bool isBreak;
    instruction(){}
    instruction( int i, int addr, bool doneBreak): isBreak( false )
    {
        unsigned int ui = (unsigned int)  i;
        this->addr = addr;
        intVal = i;
        v =  ui >> 31;
        opcode = ui >> 26;
        rs = ui << 6 >> 27;
        rt = ui <<11 >> 27;
        rd = ui << 16 >> 27;
        imm = i << 16 >> 16;
        target = (imm << 2);
        jTarget = ui <<6 >> 4;
        shamt = ui << 21 >> 27;
        func = ui <<26 >> 26;
        if( doneBreak ){
            stringstream ss;
            ss << "\t"<<addr << "\t" << i;
            out = bitset<32>( i ).to_string() + ss.str();
        } else {
            bitstr = bitset<32>(i).to_string();
            bitstr = bitstr.substr(0,1) + " " + bitstr.substr(1,5) + " " + bitstr.substr(6,5) + " "
                + bitstr.substr(11, 5 ) + " " + bitstr.substr(16, 5) + " "
                + bitstr.substr( 21, 5 ) + " " + bitstr.substr(26, 6);
            {
                stringstream ss;
                ss << addr;
                out = bitstr + "\t" + ss.str() + "\t";
            }

            if( v == 0 ){
                out = out + "Invalid Instruction";
                istr = "Invalid Instruction";
            }
            else if( ui == 2147483648 ){
                stringstream ss;
                ss << "NOP";
                out = out + ss.str();
                istr = ss.str();
                        }
            else if( opcode == 40 ){
                stringstream ss;
                ss << "ADDI\tR" << rt << ", R" << rs << ", #" << imm;
                out = out + ss.str();
                istr = ss.str();
                dest = rt;
                src1 = rs;
                src2 = rs;
            }
            else if( opcode == 43 ){
                stringstream ss;
                ss << "SW\tR" << rt << ", " <<imm << "(R" << rs << ")";
                out = out + ss.str();
                istr = ss.str();
                
                src1 = rs;
                src2 = rt;
            }
            else if( opcode == 35 ){
                stringstream ss;
                ss << "LW\tR" << rt << ", " <<imm << "(R" << rs << ")";
                out = out + ss.str();
                istr = ss.str();
                dest = rt;
                src1 = rs;
                src2 = rs;
            }
            else if( opcode == 33 ){
                stringstream ss;
                ss << "BLTZ\tR" << rs << ", #" <<target;
                out = out + ss.str();
                istr = ss.str();
                //dest = rt;
                src1 = rs;
                src2 = rs;
            }
            else if( opcode == 32 && func == 0 ){
                stringstream ss;
                ss << "SLL\tR" << rd << ", R" << rt << ", #" << shamt;
                out = out + ss.str();
                istr = ss.str();
            }
            else if( opcode == 32 && func == 2 ){
                stringstream ss;
                ss << "SRL\tR" << rd << ", R" << rt << ", #" << shamt;
                out = out + ss.str();
                istr = ss.str();
            }
            else if( opcode == 32 && func == 34 ){
                stringstream ss;
                ss << "SUB\tR" << rd << ", R" << rs << ", R" << rt;
                out = out + ss.str();
                istr = ss.str();
            }
            else if( opcode == 32 && func == 32 ){
                stringstream ss;
                ss << "ADD\tR" << rd << ", R" << rs << ", R" << rt;
                out = out + ss.str();
                istr = ss.str();
            }
            else if( opcode == 32 && func == 10 ){
                stringstream ss;
                ss << "MOVZ\tR" << rd << ", R" << rs << ", R" << rt;
                out = out + ss.str();
                istr = ss.str();
            }
            else if( opcode == 60 && func == 2 ){
                stringstream ss;
                ss << "MUL\tR" << rd << ", R" << rs << ", R" << rt;
                out = out + ss.str();
                istr = ss.str();
            }
            else if( opcode == 32 && func == 8 ){
                stringstream ss;
                ss << "JR\tR" << rs;
                out = out + ss.str();
                istr = ss.str();
            }
            else if( opcode == 34){
                stringstream ss;
                ss << "J\t#" << jTarget;
                out = out + ss.str();
                istr = ss.str();

            }
            else if( opcode == 32 && func == 13 ){
                stringstream ss;
                ss << "BREAK";
                istr = ss.str();
                out = out + ss.str();
                isBreak = true;
            }
            else {
                printf ("Opcode %i func %i not implemented\n", opcode, func);
                print();
                exit(0);
            }
        }

        
    }

    void print(){
        printf( "%s\n%s v %i; rs %i, rt %i, rd %i, shamt %i, func %i, opcode %i, func %i, imm %i, target %i, intVal %i, addr %i\n",
                out.c_str(),bitstr.c_str(), v, rs, rt, rd, shamt, func, opcode, func,imm, target, intVal, addr);
    }
};

string printState( const int R[], const int PC, const int cycle, unordered_map< int, instruction> & MEM,
          const int breakAddr, const int lastAddr,  int preIssue[], int preALU[], int preMEM[] ){
    std::ios oldState(nullptr);
    stringstream ss1;
    oldState.copyfmt(ss1);
    instruction I = MEM[PC];
    ss1<< "--------------------\ncycle:"<<cycle << endl << endl;
    ss1<< "Pre-Issue Buffer:" << endl <<
    "   Entry 0:    " << MEM[preIssue[0]].istr << endl <<
    "   Entry 1:    " << MEM[preIssue[1]].istr << endl <<
    "   Entry 2:    " << MEM[preIssue[2]].istr << endl <<
    "   Entry 3:    " << MEM[preIssue[3]].istr << endl;
    
    ss1<< "Pre_ALU Queue:" << endl <<
    "   Entry 0:    " << MEM[preALU[0]].istr << endl <<
    "   Entry 1:    " << MEM[preALU[1]].istr << endl;
    
    ss1<< "Post_ALU Queue:" << endl <<
    "   Entry 0:    " << MEM[postALU.instr].istr << endl;
    
    ss1<< "Pre_MEM Queue:" << endl <<
    "   Entry 0:    " << MEM[preMEM[0]].istr << endl <<
    "   Entry 1:    " << MEM[preMEM[1]].istr << endl;
    
    ss1<< "Post_MEM Queue:" << endl <<
    "   Entry 0:    " << MEM[postMEM.instr].istr << endl << endl;
    
    ss1 << "registers:";
        for( int i = 0; i < 32; i++ ) {
            if(i%8 == 0){
                ss1 << "\nr"<<std::setfill('0') << std::setw(2)<< i;
                std::cout.copyfmt( oldState );
            }
            ss1 << "\t" <<R[i];
        }
        ss1 << "\n\ndata:";
        for( int i = breakAddr +4; i < lastAddr; i+=4 ) {
            if( ((i-breakAddr -4)/4) % 8 == 0 )
                ss1<< "\n"<< i <<":";
            ss1<< "\t" <<  MEM[i].intVal;
        }
        ss1<<"\n";
    return ss1.str();

}

int main()
{
    unordered_map< int, instruction> MEM;
    bool doneBreak = false;
    int breakAddr = 0;
    int lastAddr = 0;
    char buffer[4];
    int i;
    char * iPtr;
    iPtr = (char*)(void*) &i;
    int addr = 96;
    int FD = open("t1.bin" , O_RDONLY);
    printf( "filename: %s", "t1.bin");
    cout << endl;
    int amt = 4;
    while( amt != 0 )
    {
        amt = read(FD, buffer, 4);
        if( amt == 4)
        {
            iPtr[0] = buffer[3];
            iPtr[1] = buffer[2];
            iPtr[2] = buffer[1];
            iPtr[3] = buffer[0];
            //cout << "i = " <<hex<< i << endl;
        }
        instruction I( i, addr, doneBreak );
        MEM[addr] = I;
        if( I.isBreak ){
            doneBreak = true;
            breakAddr = addr;
        }
        addr +=4;
    }
    lastAddr = addr-4;
    
    for( int i = 96; i < lastAddr; i+=4 )
        cout << MEM[i].out << endl;
    
    // make a register file and processor state elements
    struct processorState{
        int R[32] ={0};
        int PC = 96;
        int PCcounter = PC;
        int cycle = 1;
        unordered_map< int, instruction> MEM;
        int preIssue[4] = {0};
        int preALU[2] = {0};
        int preMEM[2] = {0};

                
        bool XBWcheck( int reg, int issuePos ){
            if( reg <0 ) return false;
            for( int i = issuePos-1; i >=0; i-- ){
                if( reg == MEM[preIssue[i]].dest ) return true;
            }
            for( int i = 0; i < 2; i++ ){
                if( reg == MEM[preALU[i]].dest ) return true;
                if( reg == MEM[preMEM[i]].dest ) return true;
            }
            if( reg == MEM[postALU.instr].dest ) return true;
            if( reg == MEM[postMEM.instr].dest ) return true;
            return false;
        }
        
        void WB(){
            // Handle postALU
            if( postALU.instr != -1 ){
                instruction instr = MEM[postALU.instr];
                
                // Update destination register with value from ALU result
                R[instr.dest] = postALU.value;
                
                // Clear postALU buffer
                postALU.instr = -1;
                postALU.value = 0;
            }
            
            // Handle postMEM
            if( postMEM.instr != -1 ){
                instruction instr = MEM[postMEM.instr];
                
                // Update destination register with value from memory
                R[instr.dest] = postMEM.value;
                
                // Clear postMEM buffer
                postMEM.instr = -1;
                postMEM.value = 0;
            }
        }
        
        void ALU(){
            if( preALU[0] != 0 ){
                instruction instr = MEM[preALU[0]];

                // Perform ALU operation based on opcode
                if (instr.opcode == 40) { // ADDI
                    R[instr.rt] = R[instr.rs] + instr.imm;
                    postALU.instr = preALU[0]; // Update postALU buffer with preALU[0]
                    postALU.value = R[instr.rt]; // Update postALU buffer with value from register
                    preALU[0] = 0; // Clear preALU[0]
                } else if (instr.opcode == 32) { // R-type instructions
                    if (instr.func == 0) { // SLL
                        R[instr.rd] = R[instr.rt] << instr.shamt;
                        postALU.instr = preALU[0]; // Update postALU buffer with preALU[0]
                        postALU.value = R[instr.rd]; // Update postALU buffer with value from register
                        preALU[0] = 0; // Clear preALU[0]
                    } else if (instr.func == 34) { // SUB
                        R[instr.rd] = R[instr.rs] - R[instr.rt];
                        postALU.instr = preALU[0]; // Update postALU buffer with preALU[0]
                        postALU.value = R[instr.rd]; // Update postALU buffer with value from register
                        preALU[0] = 0; // Clear preALU[0]
                    } else if (instr.func == 32) { // ADD
                        R[instr.rd] = R[instr.rs] + R[instr.rt];
                        postALU.instr = preALU[0]; // Update postALU buffer with preALU[0]
                        postALU.value = R[instr.rd]; // Update postALU buffer with value from register
                        preALU[0] = 0; // Clear preALU[0]
                    }
                }
            }
        }

        
        void MEMUnit(){
            // Handle preMEM[0]
            if( preMEM[0] != 0 ){
                instruction instr = MEM[preMEM[0]];
                
                // Perform memory operation based on opcode
                switch( instr.opcode ){
                    case 35: // LW
                        R[instr.rt] = MEM[ instr.imm + R[instr.rs]].intVal;
                        postMEM.instr = preMEM[0]; // Update postMEM buffer with preMEM[0]
                        postMEM.value = R[instr.rt]; // Update postMEM buffer with value from register
                        preMEM[0] = 0; // Clear preMEM[0]
                        break;
                    case 43: // SW
                        MEM[ R[instr.rs] + instr.imm ].intVal = R[ instr.rt ];
                        preMEM[0] = 0; // Clear preMEM[0]
                        break;
                    default:
                        break;
                }
            }
        }

        void ISSUE() {
            int numIssued = 0; // Counter for number of instructions issued
            bool hazard = false;
            for (int i = 0; i < 4; i++) { // Check all 4 entries in preIssue
                instruction instr = MEM[preIssue[i]];

                // Check for hazards
                if (XBWcheck(instr.dest, i)) {
                    // Skip the current instruction and move on to the next entry
                    hazard = true;
                    continue;
                }

                bool issued = false; // Flag to track if instruction has been issued

                // If the LW or SW, send to preMEM
                if (instr.opcode == 35 || instr.opcode == 43) {
                    // Issue instruction to preALU if there is a slot available
                    for (int j = 0; j < 2; j++) {
                        if (preMEM[j] == 0) {
                            preMEM[j] = instr.addr;

                            // Shift preIssue instructions up
                            for (int q = i; q < 3; q++) {
                                preIssue[q] = preIssue[q + 1];
                            }
                            preIssue[3] = 0;
                            numIssued++;
                            issued = true;
                            break;
                        }
                    }
                } else {
                    // Issue instruction to preALU for anything else
                    for (int j = 0; j < 2; j++) {
                        if (preALU[j] == 0) {
                            preALU[j] = instr.addr;

                            // Shift preIssue instructions up
                            for (int q = i; q < 3; q++) {
                                preIssue[q] = preIssue[q + 1];
                            }
                            preIssue[3] = 0;
                            numIssued++;
                            issued = true;
                            break;
                        }
                    }
                }

                // Break out of the loop if two instructions have been issued
                if (numIssued == 2) {
                    break;
                }

                // If instruction has not been issued, and there are no slots in preALU and preMEM,
                // then it stays in preIssue until the next iteration
                if (!issued && preALU[0] != 0 && preALU[1] != 0 && preMEM[0] != 0 && preMEM[1] != 0) {
                    break;
                }
            }
        }

        void IF() {
            noBranch = true;
            if (isBreak == true) {
                return;
            }
        
            for (int i = 0; i < 2; i++) {
                instruction instr = MEM[PC];
                if (instr.opcode == 0) {
//                    PC += 4;
                }
                else if ((instr.opcode == 32) && (instr.func == 8)) {
                    //JR
                    PC = instr.rs;
                }
                else if (instr.opcode == 33) {
                    //BLTZ
                    if (R[instr.rs] < 0){
                        PC = PC + instr.target;
                        noBranch = false;
                    }
                    else {
                        PC += 4;
                    }
                }
                else if (instr.opcode == 34) {
                    //J
                    if (XBWcheck(instr.dest, i)){
                      //stall IF if there is a hazard
                        return;
                    }
                    PC = instr.jTarget;
                }
                else if ((instr.opcode == 32) && (instr.func == 13)) {
                    //BREAK
                    isBreak = 1;
                }
                
                else {
                    for (int i = 0; i < 4; i++) {
                        if (preIssue[i] == 0){
                            preIssue[i] = instr.addr;
                            break;
                        }
                    }
                }
                if (noBranch){
                    PC += 4;
                }
            }
        }
    };
    
    processorState state;
    state.MEM = MEM;
    while( true ){
        
        state.WB();
        state.ALU();
        state.MEMUnit();
        state.ISSUE();
        state.IF();

        
        instruction I = MEM[ state.PC ];
        while( I.v == 0 ){
            state.PC += 4;
            I = MEM[ state.PC ];
        }
//        state.PC += 4;
//        int nextpc = state.PC;
  
        cout << printState(  state.R, state.PC-4,  state.cycle, state.MEM,  breakAddr,  lastAddr, state.preIssue, state.preALU, state.preMEM  ) << endl;
        state.cycle ++;
//                if (state.PC == breakAddr + 4) {
//                    break;
//                }
//        state.PC = nextpc;
        if( state.cycle >= 145) break;
    };
}

