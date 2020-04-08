%% Senior Project Model Definition
% Blake Capella - 10/18/19

%Constants

M = 0.5;               %Rate of Mass traveling through system [L/min] Experimnetally determined
m_resevoir_a = 0.635;   %Mass of water in one resevoir [Kg]
c_bar = 4.183;          %specific heat of Water [KJ/Kg * K]
k_ax = 13.5;            %Heat transfer coeffecient of Resevoir & Environment [J/m^2*s*K] 
k_bx = 3.329;            %Heat transfer coeffecient of Bladder & Hand [J/m^2*s*K] 
f_ax = 0.0393;  %Contact Area of Resevoir and Environment [m^2]
f_bx = 0.0261;          %Contact Area of Bladder & Hand [m^2] (Area of top avg male hand)
C_a = m_resevoir_a*c_bar; %Heat capacity of water [KJ*K]
C_bs = 1.321;           %Heat capacity of hand [KJ/K]

%% Model with No Physical Meaning
%Creation of Transfer Functions

num_d = [1];
denom_d = [1 (M*c_bar/C_a)-(k_ax*f_ax/C_a)];
tf_dir = tf(num_d,denom_d);

num_i = [C_a (M*c_bar)];
denom_i = [C_bs k_bx*f_bx];
tf_indir = tf(num_i,denom_i);

%Cascade TFs
sys_tf = tf_dir*tf_indir

%System Characteristics
%  Closed Loop Bandwidth
tf_bw = bandwidth(feedback(sys_tf,1))
%  Pzmap and step
figure(1)
pzmap(sys_tf)
title("System Pole-Zero Plot")
grid on
figure(2)
step(sys_tf)
title("System Step Response")

%Convert to SS
[A,B,C,D] = tf2ss([sys_tf.Numerator{1,:}],[sys_tf.Denominator{1,:}]);

%Controllability Check
co = rank(ctrb(A,B));

%Observability Check
ob = rank(obsv(A,C));

%A matrix Rank
rn_A = rank(A);

%% State Space with Physical Meaning
%Paper Calculations for Matrix Defenition
A_real = [((-M*c_bar) - (k_ax*f_ax))/C_a 0; (2*M*c_bar + k_ax*f_ax)/C_bs (-k_bx*f_bx)/C_bs];
B_real = [(1/C_a); (-1/C_bs)];
C_real = [0 1];
D_real = 0;

%Sampling Time
%Ts = 1/100;

%Model Synthesis
G = ss(A_real, B_real, C_real, D_real);
[Gn, Gd] = ss2tf(A_real, B_real, C_real, D_real);
tf_g = tf(Gn,Gd);

%System Characteristics
%  Pole Zero Map
figure(3)
pzmap(G)
title("Manual Realization Pole Zero Mapping")
grid on
%  Step Response
figure(4)
step(G)
title("Manual Realization Step Response")

%Controllability Check
A_rank = rank(A_real);
co = rank(ctrb(A_real,B_real));
if (A_rank == co)
    co_bool = 1;
    
else
    co_bool = 0;
end

%Observability Check
ob = rank(obsv(A_real,C_real));
if (A_rank == ob)
    ob_bool = 1;
    
else
    ob_bool = 0;
end

%% Pole Placement for Simulink 
K = place(A,B,[-0.21 -0.22]);
L_t = place(A.',C.',[-2.1 -2.2]);
L = L_t.';

%% Simulink Plot
yyaxis left
plot(simout_ref)
ylabel("Power (W)")
hold on
yyaxis right
plot(simout_y)
ylabel("Temperature Change (Celcius)")
legend("Reference","Temperature at Hand")
title("Simulink Step Response")
xlabel("Seconds")
hold off