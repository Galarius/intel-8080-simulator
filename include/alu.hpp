//
//  alu.hpp
//
//  Created by Ilia Shoshin on 28.09.24.
//

#pragma once

#include <systemc>

namespace sim {

/*
 * Arithmetic and Logic Unit (ALU)
 *
 * SSS DDD	2	1	0	CC	ALU	                        RP	
 *    B	    0	0	0	NZ	ADD ADI (A ← A + arg)	    BC
 *    C	    0	0	1	Z	ADC ACI (A ← A + arg + Cy)	DE
 *    D	    0	1	0	NC	SUB SUI (A ← A - arg)	    HL
 *    E	    0	1	1	C	SBB SBI (A ← A - arg - Cy)	SP or PSW
 *    H	    1	0	0	PO	ANA ANI (A ← A ∧ arg)
 *    L	    1	0	1	PE	XRA XRI (A ← A ⊻ arg)
 *    M	    1	1	0	P	ORA ORI (A ← A ∨ arg)
 *    A	    1	1	1	N	CMP CPI (A - arg)
 * 
 * TODO: add support for operations (ADI, ACI, SUI, SBI, ANI, XRI, ORI, CPI) that use immediate value
 */
class ALU final : sc_core::sc_module {

public:
    static const uint8_t OP_ADD; // ADD ; A ← A + arg
    static const uint8_t OP_ADC; // ADD ; A ← A + arg + Cy
    static const uint8_t OP_SUB; // SUB ; A ← A - arg
    static const uint8_t OP_SBB; // SBB ; A ← A - arg - Cy
    static const uint8_t OP_ANA; // SBB ; A ← A ∧ arg
    static const uint8_t OP_XRA; // XRA ; A ← A ⊻ arg
    static const uint8_t OP_ORA; // ORA ; A ← A ∨ arg
    static const uint8_t OP_CMP; // CMP ; A - arg

    static const uint8_t FLAG_IDX_ZERO;
    static const uint8_t FLAG_IDX_CARRY;
    static const uint8_t FLAG_IDX_SIGN;
    static const uint8_t FLAG_IDX_PARITY;
    static const uint8_t FLAG_IDX_AUX_CARRY;
    
    // Ports
    sc_core::sc_in<sc_dt::sc_uint<8>> inA;          // 8-bit input (operand)
    sc_core::sc_in<sc_dt::sc_uint<8>> inArg;        // 8-bit input (operand)
    sc_core::sc_in<sc_dt::sc_uint<4>> inOpCode;     // Control signal for operation
    sc_core::sc_out<sc_dt::sc_uint<8>> outResult;   // 8-bit output
    sc_core::sc_out<sc_dt::sc_uint<5>> outFlags;    // 5-bit flag output (zero, carry, sign, parity, and auxiliary carry)

    void execute();

    ALU(sc_core::sc_module_name name);
};

} // namespace sim
