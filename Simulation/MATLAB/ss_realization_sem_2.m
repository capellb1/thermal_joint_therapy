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

%Creation of Transfer Functions
num_i = [C_a (M*c_bar)];
denom_i = [C_bs k_bx*f_bx];
tf_indir = tf(num_i,denom_i);

figure(1)
pzmap(tf_indir)
title("System Pole-Zero Plot")
grid on
figure(2)
step(tf_indir)
title("System Step Response")

%Convert to SS
[A,B,C,D] = tf2ss(tf_indir.Numerator{1,:},tf_indir.Denominator{1,:});

%Controllability Check
co = rank(ctrb(A,B));

%Observability Check
ob = rank(obsv(A,C));