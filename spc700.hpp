#ifndef _SPC700_H
#define _SPC700_H

#include "common.h"
#include "aram.hpp"

#include <map>
#include <functional>

class SPC700 {
public:
    void init();
    bool clock();
private:
    byte A;
    byte X;
    byte Y;
    byte SP;
    byte PC;
    byte PSW;

    //
	// operations (using 6502 syntax where possible)
	//

    // todo: update comments to describe _X, _DP, and _X_POSTINC

    void LDA();
    void LDA_X(); void LDA_X_POSTINC();
    void LDX(); void LDY();

    void STA();
    void STA_X(); void STA_X_POSTINC();
    void STX(); void STY();

    void TXA(); void TYA();
    void TAX(); void TAY();
    void TSX(); void TXS();
    void MOV_DP();

    void ADC();
    void ADC_X(); void ADC_DP();

    void SBC();
    void SBC_X(); void SBC_DP();

    void CMP();
    void CMP_X(); void CMP_DP();

    void CPX(); void CPY();

    void AND();
    void AND_X(); void AND_DP();

    void ORA();
    void ORA_X(); void ORA_DP();

    void EOR();
    void EOR_X(); void EOR_DP();

    void INC();
    void INCA(); void INX(); void INY();

    void DEC();
    void DECA(); void DEX(); void DEY();

    void ASL();
    void ASLA();

    void LSR();
    void LSRA();

    void ROL();
    void ROLA();

    void ROR();
    void RORA();

    void XCN();

    void MOVW();
    void MOVW_DP();

    void INCW(); void DECW();

    void ADDW(); void SUBW(); void CMPW();

    void MUL(); void DIV();

    void DAA(); void DAS();

    void BRA();
    void BEQ(); void BNE();
    void BCS(); void BCC();
    void BVS(); void BVC();
    void BMI(); void BPL();

    void BBS0(); void BBS1(); void BBS2(); void BBS3();
    void BBS4(); void BBS5(); void BBS6(); void BBS7();

    void BBC0(); void BBC1(); void BBC2(); void BBC3();
    void BBC4(); void BBC5(); void BBC6(); void BBC7();

    void CBNE();
    void CBNE_X();
    void DBNZ();
    void DBNZ_Y();

    void JMP();

    void JSR();
    void PCALL();
    void TCALL();

    void BRK(); void RTS(); void RTI();

    void PHA(); void PHX(); void PHY(); void PHP();
    void PLA(); void PLX(); void PLY(); void PLP();

    void SMB0(); void SMB1(); void SMB2(); void SMB3();
    void SMB4(); void SMB5(); void SMB6(); void SMB7();

    void RMB0(); void RMB1(); void RMB2(); void RMB3();
    void RMB4(); void RMB5(); void RMB6(); void RMB7();

    void TSB(); void TRB();

    void AND1(); void AND1_N();
    void OR1(); void OR1_N();
    void EOR1();
    void NOT1();
    void MOV1(); void MOV1_REVERSE();

    void CLC(); void SEC(); void NOTC();
    void CLV();
    void CLRP(); void SETP();
    void CLI(); void SEI();

    void NOP() {}
    void WAI();
    void STP();

    //
    // addressing modes
    //

    // immediate
    void IMM();

    // direct page
    void DP();
    void DP_DP(); void DP_IMM();

    // absolute
    void ABS();

    // relative offset
    void REL();

    // direct page relative
    void DP_REL_X(); void DP_REL_Y();

    // special mode for bit operations:
    // high 3 bits are target bit, remaining 15 bits are address to fetch data from
    void MEM_BIT();

    // implied
    void IMP() {}

    // flags
	union {
		struct {
			char c : 1;
			char z : 1;
			char i : 1;
			char d : 1;
			char x : 1;
			char m : 1;
			char v : 1;
			char n : 1;
		} bits;
		char full;
	} status;

    byte fetched;
    twobyte dest_addr;
    // used for testing/setting specified bit of data
    byte data_bit;

    typedef struct {
		std::string name;

		std::function<void()> op;
		std::function<void()> mode;
		std::function<byte()> cycleCount;
	} instruction;

    std::map<byte, instruction> ops {

    };

    SNES_ARAM ram;
};

#endif //_SPC700_H