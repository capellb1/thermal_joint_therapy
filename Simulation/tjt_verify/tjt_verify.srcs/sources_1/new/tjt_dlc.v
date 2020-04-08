`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: Blake Capella
// 
// Create Date: 02.03.2020 12:42:25
// Design Name: 
// Module Name: tjt_dlc
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module tjt_dlc(
    input Pd,
    input B,
    input time_h,
    input time_c,
    input time_d,
    input S,
    output P1,
    output P2,
    output G1,
    output G2,
    output G3,
    output G4
    );
    
   assign P1 = (!S)&&(!time_d)&&(P1 || B);
   assign P2 = (!Pd)&&(P2 || B);
   assign G1 = (!time_h)&&(!S)&&(G1 || B || (time_c && !S));
   assign G2 = (!time_c)&&(!S)&&(!time_d)&&(G2 || (time_h && !S));
   assign G3 = (!(time_h && !S))&&(!Pd)&&(G3 || B || (time_c && !S));
   assign G4 = (!(time_c && !S))&&(!Pd)&&(G4 || (time_h && !S));

endmodule
