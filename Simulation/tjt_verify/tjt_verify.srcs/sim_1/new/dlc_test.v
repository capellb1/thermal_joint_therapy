`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Engineer: Blake Capella
// 
// Create Date: 02.03.2020 12:56:41
// Design Name: Thermal Joint Therapy
// Module Name: dlc_test
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


module dlc_test;
        
        //input
        reg Pd;
        reg time_c;
        reg time_h;
        reg time_d;
        reg B;
        reg S; 
        
        //output
        wire P1;
        wire P2;
        wire G1;
        wire G2;
        wire G3;
        wire G4; 
        
        tjt_dlc uut(
        .Pd(Pd),
        .B(B),
        .time_h(time_h),
        .time_c(time_c),
        .time_d(time_d),
        .S(S),
        .P1(P1),
        .P2(P2),
        .G1(G1),
        .G2(G2),
        .G3(G3),
        .G4(G4)
        );
        
        initial begin
            //Begin with no inputs;
            Pd = 0;
            time_c = 0;
            time_h = 0;
            time_d = 0;
            B = 0;
            S = 0; 
            #1  
            B=1; //Begin -> Heat
            #1
            time_h = 1; // -> Cool
            B = 0;
            #1
            time_c = 1; // -> heat
            time_h = 0;
            #1
            time_h = 1; // -> cool
            time_c = 0;
            #1
            time_d = 1; // -> evac (routine)
            time_h = 0;
            #1
            Pd = 1; //-> stdby
            time_d = 0;
            #1
            B = 1; // -> heat
            Pd = 0;
            #1
            S = 1; // -> evacuate (emergency)
            B = 0;
            #1
            Pd = 1; // -> stdby
            S = 0;
            #1
            B=1; // -> heat
            Pd = 0;
            #1
            time_h = 1; // -> cool
            B = 0;
            #1
            time_h = 0; 
            S = 1;
            #1
            Pd = 1;
            S = 0;
        end
        initial #100 $finish;
endmodule
